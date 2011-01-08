#include "sshconnection.h"

#ifdef Q_OS_WIN
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
	void closesocket(int socket) { close(socket); }
#endif

#include <QDebug>
#include <QTime>
#include <QSettings>
#include <QMessageBox>
#include "main/global.h"

QMap<QString, QByteArray> SshConnection::sKnownHostKeys;

void SshConnection::initializeLib()
{
#ifdef Q_OS_WIN
	//	Initialize winsock
	WSADATA data;
	if (WSAStartup(0x22, &data) != 0)
		throw(QString("Failed to initialize WinSock!"));
#endif
	//	Initialize libssh2
	if (libssh2_init(0) != 0)
		throw(QString("Failed to initialize SSH library!"));

	//	Load any saved hostkey pairs
	QSettings settings;
	int count = settings.beginReadArray(ntr("hostkeys"));
	for (int i = 0; i < count; i++)
	{
		settings.setArrayIndex(i);
		QString hostname = settings.value(ntr("hostname")).toString();
		QByteArray key = settings.value(ntr("hostkey")).toByteArray();
		sKnownHostKeys.insert(hostname, key);
	}
}

SshConnection::SshConnection()
	: mSocket(-1), mSession(0), mChannel(0)
{}

SshConnection::~SshConnection()
{
	disconnect();
}

void SshConnection::connect(const char* host, int port)
{
	//	nslookup the hostname
	struct hostent* server = gethostbyname(host);
	if (server == NULL)
		throw(QObject::tr("Failed to find host"));

	//	Create a socket & connect
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < -1)
		throw(QObject::tr("Failed to create a socket!"));

	// Disable SIGPIPE on this socket
#ifndef Q_OS_WIN
	int set = 1;
	setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif

	//	Cycle through all the IP addresses returned by the nslookup, try to connect to each
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	int tryAddress = 0;
	while (mSocket < 0 && server->h_addr_list[tryAddress] != 0)
	{
		sin.sin_addr.s_addr = *(u_long*)server->h_addr_list[tryAddress];
		int rc = ::connect(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
		if (rc == 0)
			mSocket = sock;
		tryAddress++;
	}
	if (mSocket < 0)
		throw(QObject::tr("Failed to connect to host"));

	//	Create an SSH2 session
	mSession = libssh2_session_init();
	libssh2_session_flag(mSession, LIBSSH2_FLAG_SIGPIPE, 1);
	if (libssh2_session_startup(mSession, mSocket))
	{
		disconnect();
		throw(QObject::tr("Failed to set up SSH session"));
	}

	//	Fetch the remote host's fingerprint
	mServerFingerprint = QByteArray(libssh2_hostkey_hash(mSession, LIBSSH2_HOSTKEY_HASH_SHA1), 20);
}

void SshConnection::disconnect()
{
	if (mChannel)
	{
		libssh2_channel_close(mChannel);
		libssh2_channel_wait_closed(mChannel);
		libssh2_channel_free(mChannel);
		mChannel = NULL;
	}

	if (mSession)
	{
		libssh2_session_disconnect(mSession, "Closing connection");
		libssh2_session_free(mSession);
		mSession = NULL;
	}

	if (mSocket > -1)
	{
		closesocket(mSocket);
		mSocket = -1;
	}
}

SshConnection::AuthMethods SshConnection::getAuthenticationMethods(const char* username)
{
	AuthMethods methods = (AuthMethods)0;
	char* userauthlist = libssh2_userauth_list(mSession, username, strlen(username));
	if (strstr(userauthlist, "password") != NULL)
		methods = (AuthMethods)(methods | Password);
	if (strstr(userauthlist, "keyboard-interactive") != NULL)
		methods = (AuthMethods)(methods | KeyboardInteractive);
	if (strstr(userauthlist, "publickey") != NULL)
		methods = (AuthMethods)(methods | PublicKey);

	qDebug() << userauthlist;

	return methods;
}

bool SshConnection::authenticatePassword(const char* username, const char* password)
{
	int rc = libssh2_userauth_password(mSession, username, password);
	if (rc == LIBSSH2_ERROR_AUTHENTICATION_FAILED)
		return false;

	if (rc)
		throw(QObject::tr("Connection dropped!"));

	createChannel();
	return true;
}

bool SshConnection::authenticateAgent(const char* username)
{
	struct libssh2_agent_publickey *identity, *prevIdentity = NULL;
	LIBSSH2_AGENT* agent = NULL;
	int rc;
	bool success = false;

	try
	{
		//	Initialize SSH agent code
		agent = libssh2_agent_init(mSession);
		if (!agent) throw(QObject::tr("Failed to call libssh2_agent_init!"));

		//	Connect to the SSH agent
		rc = libssh2_agent_connect(agent);
		if (rc) throw(QObject::tr("Failed to connect to SSH agent!"));

		//	Prepare a list of identities managed by the SSH agent
		rc = libssh2_agent_list_identities(agent);
		if (rc) throw(QObject::tr("Failed to retrieve identities from SSH agent!"));

		while (1)
		{
			//	Get an identity
			rc = libssh2_agent_get_identity(agent, &identity, prevIdentity);
			if (rc) throw(rc == 1 ? QObject::tr("No identities stored in SSH agent accepted!") : QObject::tr("Failed to receive identity from SSH agent!"));

			//	Try an identity
			qDebug() << "Trying key: " << identity->comment;
			rc = libssh2_agent_userauth(agent, username, identity);
			if (rc)
				qDebug() << "Authentication with key " << identity->comment << " rejected.";
			else
			{
				qDebug() << "Authentication with key " << identity->comment << " accepted!";
				success = true;
				break;
			}

			prevIdentity = identity;
		}
	}
	catch (const QString& error)
	{
		qDebug() << "Failed to authenticate via SSH agent: " << error;
	}

	if (agent)
	{
		libssh2_agent_disconnect(agent);
		libssh2_agent_free(agent);
	}

	if (success)
		createChannel();

	return success;
}

void SshConnection::createChannel()
{
	qDebug() << "Creating an SSH channel...";

	//	Create a channel
	mChannel = libssh2_channel_open_session(mSession);
	if (!mChannel)
		throw(QString("Failed to create SSH channel!"));

	if (libssh2_channel_request_pty(mChannel, "vanilla"))
		throw(QString("Failed to set vanilla pty mode"));

	//	Start up a shell
	if (libssh2_channel_shell(mChannel))
		throw(QString("Failed to create a shell"));

	//	Turn off blocking
	//libssh2_channel_set_blocking(mChannel, 0);

	//	Shove a message down the line that should change the user's prompt...
	const char* command = "export PS1=\\%-remoted-\\%\n";
	libssh2_channel_write(mChannel, command, strlen(command));
	readToPrompt();

	//	Shove a message down the line that should turn stty echo off
	command = "stty -echo\n";
	libssh2_channel_write(mChannel, command, strlen(command));
	readToPrompt();

	qDebug() << "SSH Shell session created!";
}

QByteArray SshConnection::readToPrompt()
{
	return readUntil(SSH_PROMPT);
}

QByteArray SshConnection::readLine()
{
	return readUntil("\n");
}

QByteArray SshConnection::readUntil(const char* marker)
{
	int totalReceived = 0;
	while (1)
	{
		int markerIndex = mReadBuffer.indexOf(marker);
		if (markerIndex > -1)
		{
			QByteArray result = mReadBuffer.left(markerIndex);
			mReadBuffer = mReadBuffer.right(mReadBuffer.size() - (markerIndex + strlen(marker)));
			return result;
		}

		int rc = libssh2_channel_read(mChannel, mTmpBuffer, SSH_BUFFER_SIZE);
		if (rc > 0)
		{
			totalReceived += rc;
			// qDebug() << QByteArray(mTmpBuffer, rc);
			mReadBuffer.append(mTmpBuffer, rc);
		}
		else if (rc < 0)
			throw(QString("Error while receiving from remote host: ") + getLastError(rc));
		else if (libssh2_channel_eof(mChannel))
			throw(QString("Connection closed by remote host."));
	}
}

QString SshConnection::getLastError(int rc)
{
	char* errorBuffer;
	int errorLength;
	libssh2_session_last_error(mSession, &errorBuffer, &errorLength, 0);

	QString error;
	error.sprintf("%d - %s", rc, errorBuffer);
	return error;
}

QByteArray SshConnection::execute(const char* command)
{
	// qDebug() << "SENDING: " << command;
	int len = strlen(command);
	int rc = libssh2_channel_write(mChannel, command, len);
	if (rc != len)
		throw(QString("Error while sending to remote host: ") + getLastError(rc));

	return readToPrompt();
}

void SshConnection::writeData(const char* data, int length)
{
	// qDebug() << "SENDING: " << data;
	int rc = libssh2_channel_write(mChannel, data, length);
	// qDebug() << "Sent bytes: " << rc;
	if (rc != length)
		throw(QString("Error while sending to remote host: ") + getLastError(rc));
}

void SshConnection::sendEof()
{
	libssh2_channel_send_eof(mChannel);
}

void SshConnection::writeFile(const char* remoteFilename, const char* data, int length)
{
	LIBSSH2_CHANNEL* tmpChannel = libssh2_scp_send(mSession, remoteFilename, 0777, length);
	if (!tmpChannel)
	{
		char *errmsg;
		int errlen;
		libssh2_session_last_error(mSession, &errmsg, &errlen, 0);
		throw(QString(errmsg));
	}

	if (libssh2_channel_write(tmpChannel, data, length) < 0)
		throw(QString("Error sending file!"));

	libssh2_channel_send_eof(tmpChannel);

	libssh2_channel_wait_closed(tmpChannel);
	libssh2_channel_free(tmpChannel);
}

QByteArray SshConnection::readFile(const char* filename, ISshConnectionCallback* callback)
{
	struct stat fileInfo;
	LIBSSH2_CHANNEL* tmpChannel = libssh2_scp_recv(mSession, filename, &fileInfo);
	if (!tmpChannel)
		throw(QString("Failed to open remote file!"));

	QByteArray fileContent;
	while (fileContent.length() < fileInfo.st_size)
	{
		int amount = SSH_BUFFER_SIZE;

		//	Never ask for more bytes than there are left in the file.
		if (fileInfo.st_size - fileContent.length() < amount)
			amount = fileInfo.st_size - fileContent.length();

		int rc = libssh2_channel_read(tmpChannel, mTmpBuffer, amount);
		if (rc > 0)
			fileContent.append(mTmpBuffer, rc);
		else if (rc < 0)
			throw(QString("Error while receiving remote file content: ") + getLastError(rc));

		if (callback != NULL)
			callback->fileOpenProgress((fileContent.length() * 100) / fileInfo.st_size);
	}

	return fileContent;
}

void SshConnection::saveFingerprint(const QString &host, const QByteArray &fingerprint)
{
	sKnownHostKeys.insert(host, fingerprint);
	saveKnownHostKeys();
}

void SshConnection::saveKnownHostKeys()
{
	QSettings settings;

	int index = 0;
	settings.beginWriteArray(ntr("hostkeys"));
	QStringList keys = sKnownHostKeys.keys();
	foreach (QString key, keys)
	{
		settings.setArrayIndex(index++);
		settings.setValue(ntr("hostname"), key);
		settings.setValue(ntr("hostkey"), sKnownHostKeys.value(key));
	}
	settings.endArray();
}

















