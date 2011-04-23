#include "rawsshconnection.h"

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
#include <QThread>
#include "main/global.h"

#define DEFAULT_PERMISSIONS LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR | LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH

QMap<QString, QByteArray> RawSshConnection::sKnownHostKeys;

void RawSshConnection::initializeLib()
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

void RawSshConnection::cleanup()
{
    libssh2_exit();
}

RawSshConnection::RawSshConnection()
	: mSocket(-1), mSession(0)
{}

RawSshConnection::~RawSshConnection()
{
	disconnect();
}

void RawSshConnection::connect(const char* host, int port)
{
	disconnect();
	qDebug() << "Starting to connect... ";

	//	nslookup the hostname
	struct hostent* server = gethostbyname(host);
	if (server == NULL)
		throw(QObject::tr("Failed to find host"));

	//	Create a socket & connect
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < -1)
		throw(QObject::tr("Failed to create a socket!"));

	// Disable SIGPIPE on this socket
#ifdef Q_OS_MAC
	int set = 1;
	setsockopt(sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif

	//	Cycle through all the IP addresses returned by the nslookup, try to connect to each
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	int tryAddress = 0;
	mSocket = -1;
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

void RawSshConnection::disconnect()
{
	//	TODO: Close all channels
	if (mSession)
	{
		libssh2_session_disconnect(mSession, "Closing connection");
		libssh2_session_free(mSession);
		qDebug() << "Session free called on " << (void*)mSession;
		mSession = NULL;
	}

	if (mSocket > -1)
	{
		closesocket(mSocket);
		mSocket = -1;
	}
}

RawSshConnection::AuthMethods RawSshConnection::getAuthenticationMethods(const char* username)
{
	AuthMethods methods = (AuthMethods)0;
	char* userauthlist = libssh2_userauth_list(mSession, username, strlen(username));
	if (strstr(userauthlist, "password") != NULL)
		methods = (AuthMethods)(methods | Password);
	if (strstr(userauthlist, "keyboard-interactive") != NULL)
		methods = (AuthMethods)(methods | KeyboardInteractive);
	if (strstr(userauthlist, "publickey") != NULL)
		methods = (AuthMethods)(methods | PublicKey);

	return methods;
}

bool RawSshConnection::authenticatePassword(const char* username, const char* password)
{
	int rc = libssh2_userauth_password(mSession, username, password);
	if (rc == LIBSSH2_ERROR_AUTHENTICATION_FAILED)
		return false;

	if (rc)
		throw(QObject::tr("Connection dropped!"));

	return true;
}

bool RawSshConnection::authenticateKeyFile(const char* filename, const char* username, const char* password, bool* keyRejected)
{
	qDebug() << filename;
	int rc = libssh2_userauth_publickey_fromfile_ex(mSession, username, strlen(username),
												 NULL, filename, password);

	if (rc == 0)
	{
		*keyRejected = false;
		return true;
	}

	*keyRejected = (rc == LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED || rc == LIBSSH2_ERROR_FILE);
	return false;
}

bool RawSshConnection::authenticateAgent(const char* username)
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

	return success;
}

RawSshConnection::Channel* RawSshConnection::createFTPChannel()
{
	LOCK_MUTEX(mAccessMutex);

	Channel* channel = new Channel();
	channel->sftpSession = libssh2_sftp_init(mSession);
	if (!channel)
	{
		UNLOCK_MUTEX(mAccessMutex);
		delete channel;
		throw(QObject::tr("Failed to create SFTP channel!"));
	}

	UNLOCK_MUTEX(mAccessMutex);

	return channel;
}

RawSshConnection::Channel* RawSshConnection::createShellChannel()
{
	LOCK_MUTEX(mAccessMutex);

	Channel* channel = new Channel();
	channel->handle = libssh2_channel_open_session(mSession);
	if (!channel)
	{
		UNLOCK_MUTEX(mAccessMutex);
		delete channel;
		throw(QObject::tr("Failed to create SSH channel!"));
	}

	if (libssh2_channel_request_pty(channel->handle, "vanilla"))
	{
		UNLOCK_MUTEX(mAccessMutex);
		delete channel;
		throw(QObject::tr("Failed to set vanilla pty mode"));
	}

	//	Start up a shell
	if (libssh2_channel_shell(channel->handle))
	{
		UNLOCK_MUTEX(mAccessMutex);
		delete channel;
		throw(QObject::tr("Failed to create a shell"));
	}

	//	Shove a message down the line that should change the user's prompt...
	const char* command = " export PS1=\\%-ponyedit-\\%\n";
	libssh2_channel_write(channel->handle, command, strlen(command));
	UNLOCK_MUTEX(mAccessMutex);
	readToPrompt(channel);

	//	Shove a message down the line that should turn stty echo off
	command = " stty -echo\n";
	LOCK_MUTEX(mAccessMutex);
	libssh2_channel_write(channel->handle, command, strlen(command));
	UNLOCK_MUTEX(mAccessMutex);
	readToPrompt(channel);

	return channel;
}

QByteArray RawSshConnection::readToPrompt(Channel* channel)
{
	return readUntil(channel, SSH_PROMPT);
}

QByteArray RawSshConnection::readLine(Channel* channel)
{
	return readUntil(channel, "\n");
}

QByteArray RawSshConnection::readUntil(Channel* channel, const char* marker)
{
	int totalReceived = 0;
	while (1)
	{
		int markerIndex = channel->readBuffer.indexOf(marker);
		if (markerIndex > -1)
		{
			QByteArray result = channel->readBuffer.left(markerIndex);
			channel->readBuffer = channel->readBuffer.right(channel->readBuffer.size() - (markerIndex + strlen(marker)));
			return result;
		}

		LOCK_MUTEX(mAccessMutex);
		qDebug() << "libssh2_channel_read in thread " << QThread::currentThread() << ", rawsshconnection = " << (void*)this;
		int rc = libssh2_channel_read(channel->handle, channel->tmpBuffer, SSH_BUFFER_SIZE);
		UNLOCK_MUTEX(mAccessMutex);

		if (rc > 0)
		{
			totalReceived += rc;
			//qDebug() << QByteArray(channel->tmpBuffer, rc);
			channel->readBuffer.append(channel->tmpBuffer, rc);
		}
		else if (rc < 0)
			throw(QString("Error while receiving from remote host: ") + getLastError(rc));
		else
		{
			LOCK_MUTEX(mAccessMutex);
			bool eof = libssh2_channel_eof(channel->handle);
			UNLOCK_MUTEX(mAccessMutex);
			if (eof) throw(QString("Connection closed by remote host."));
		}
	}
}

QString RawSshConnection::getLastError(int rc)
{
	char* errorBuffer;
	int errorLength;

	LOCK_MUTEX(mAccessMutex);
	libssh2_session_last_error(mSession, &errorBuffer, &errorLength, 0);
	UNLOCK_MUTEX(mAccessMutex);

	QString error;
	error.sprintf("%d - %s", rc, errorBuffer);
	return error;
}

QByteArray RawSshConnection::execute(Channel* channel, const char* command)
{
	int len = strlen(command);
	LOCK_MUTEX(mAccessMutex);
	int rc = libssh2_channel_write(channel->handle, command, len);
	UNLOCK_MUTEX(mAccessMutex);
	if (rc != len)
		throw(QString("Error while sending to remote host: ") + getLastError(rc));

	return readToPrompt(channel);
}

void RawSshConnection::writeData(Channel* channel, const char* data, int length)
{
	//	Break the data up into lumps no larger than 32KB so LibSSH2 doesn't freak out
	int cursor = 0;
	while (cursor < length)
	{
		int sendLength = (length - cursor > 32000 ? 32000 : length - cursor);

		//	Lock mutex with each 32000 block
		LOCK_MUTEX(mAccessMutex);
		int rc = libssh2_channel_write(channel->handle, data+cursor, sendLength);
		UNLOCK_MUTEX(mAccessMutex);
		if (rc != sendLength)
			throw(QString("Error while sending to remote host: ") + getLastError(rc));

		cursor += 32000;
	}
}

void RawSshConnection::sendEof(Channel* channel)
{
	LOCK_MUTEX(mAccessMutex);
	libssh2_channel_send_eof(channel->handle);
	UNLOCK_MUTEX(mAccessMutex);
}

void RawSshConnection::writeFile(const char* remoteFilename, const char* data, int length)
{
	LOCK_MUTEX(mAccessMutex);
	LIBSSH2_CHANNEL* tmpChannel = libssh2_scp_send(mSession, remoteFilename, 0777, length);
	UNLOCK_MUTEX(mAccessMutex);

	if (!tmpChannel)
	{
		char *errmsg;
		int errlen;
		LOCK_MUTEX(mAccessMutex);
		libssh2_session_last_error(mSession, &errmsg, &errlen, 0);
		UNLOCK_MUTEX(mAccessMutex);
		throw(QString(errmsg));
	}

	LOCK_MUTEX(mAccessMutex);
	if (libssh2_channel_write(tmpChannel, data, length) < 0)
		throw(QString("Error sending file!"));

	libssh2_channel_send_eof(tmpChannel);

	libssh2_channel_wait_closed(tmpChannel);
	libssh2_channel_free(tmpChannel);
	UNLOCK_MUTEX(mAccessMutex);
}

QByteArray RawSshConnection::readFile(const char* filename, ISshConnectionCallback* callback)
{
	LOCK_MUTEX(mAccessMutex);

	struct stat fileInfo;
	memset(&fileInfo, 0, sizeof(fileInfo));
	LIBSSH2_CHANNEL* tmpChannel = libssh2_scp_recv(mSession, filename, &fileInfo);
	if (!tmpChannel)
	{
		UNLOCK_MUTEX(mAccessMutex);
		throw(QString("Failed to open remote file: ") + getLastError());
	}

	UNLOCK_MUTEX(mAccessMutex);

	QByteArray fileContent;
	char tmpBuffer[SSH_BUFFER_SIZE];
	while (fileContent.length() < fileInfo.st_size)
	{
		int amount = SSH_BUFFER_SIZE;

		//	Never ask for more bytes than there are left in the file.
		if (fileInfo.st_size - fileContent.length() < amount)
			amount = fileInfo.st_size - fileContent.length();

		LOCK_MUTEX(mAccessMutex);
		int rc = libssh2_channel_read(tmpChannel, tmpBuffer, amount);
		UNLOCK_MUTEX(mAccessMutex);
		if (rc > 0)
			fileContent.append(tmpBuffer, rc);
		else if (rc < 0)
			throw(QString("Error while receiving remote file content: ") + getLastError(rc));

		if (callback != NULL)
			callback->fileOpenProgress((fileContent.length() * 100) / fileInfo.st_size);
	}

	LOCK_MUTEX(mAccessMutex);
	libssh2_channel_free(tmpChannel);
	UNLOCK_MUTEX(mAccessMutex);

	return fileContent;
}

void RawSshConnection::saveFingerprint(const QString &host, const QByteArray &fingerprint)
{
	sKnownHostKeys.insert(host, fingerprint);
	saveKnownHostKeys();
}

void RawSshConnection::saveKnownHostKeys()
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

QString RawSshConnection::tidyFtpPath(const QString& path)
{
	QString tidyPath = path;
	if (tidyPath.startsWith("/~"))
		tidyPath = "." + tidyPath.mid(2);

	return tidyPath;
}

QList<Location> RawSshConnection::getFTPListing(Channel* channel, const Location& parent, bool includeHidden)
{
	QList<Location> result;
	QString realPath = tidyFtpPath(parent.getRemotePath());
	if (!realPath.endsWith('/'))
		realPath += "/";

	LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_opendir(channel->sftpSession, realPath.toLatin1());
	if (handle == NULL)
		throw(QObject::tr("Failed to open directory for reading"));

	//	Loop until reading an entry fails
	while (1)
	{
		char buffer[1024];
		char longEntry[2048];
		LIBSSH2_SFTP_ATTRIBUTES attribs;

		int rc = libssh2_sftp_readdir_ex(handle, buffer, sizeof(buffer), longEntry, sizeof(longEntry), &attribs);
		if (rc <= 0)
			break;

		//	Skip hidden and . and .. entries
		if (buffer[0] == '.')
			if (!includeHidden || buffer[1] == 0 || (buffer[2] == '.' && buffer[3] == 0)) continue;

		result.append(Location(parent, parent.getPath() + "/" + buffer,
			S_ISDIR(attribs.permissions) ? Location::Directory : Location::File, attribs.filesize,
			QDateTime::fromMSecsSinceEpoch(attribs.mtime * 1000), true, true));
	}

	libssh2_sftp_closedir(handle);
	return result;
}

QByteArray RawSshConnection::readFTPFile(Channel* channel, const Location& location, ISshConnectionCallback* callback)
{
	QString path = tidyFtpPath(location.getRemotePath());
	LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_open(channel->sftpSession, path.toAscii(), LIBSSH2_FXF_READ,	DEFAULT_PERMISSIONS);

	if (handle == NULL)
		throw(QObject::tr("Failed to open file for reading via SFTP"));

	int size = location.getSize();

	char buffer[2048];
	QByteArray data;
	while (1)
	{
		int rc = libssh2_sftp_read(handle, buffer, sizeof(buffer));
		if (rc > 0)
		{
			data.append(buffer, rc);
			if (callback && size) callback->fileOpenProgress((data.length() * 100) / size);
		}
		else if (rc == 0)
		{
			if (callback) callback->fileOpenProgress(100);
			break;
		}
		else
			throw("Failed to read remote file via SFTP");
	}

	libssh2_sftp_close(handle);
	return data;
}

void RawSshConnection::writeFTPFile(Channel* channel, const Location& location, const QByteArray& content)
{
	QString path = tidyFtpPath(location.getRemotePath());
	LIBSSH2_SFTP_HANDLE* handle = libssh2_sftp_open(channel->sftpSession, path.toAscii(),
		LIBSSH2_FXF_TRUNC | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_WRITE, DEFAULT_PERMISSIONS);

	if (handle == NULL)
		throw(QObject::tr("Failed to open file for writing via SFTP"));

	const char* ptr = content.constData();
	const char* end = content.constData() + content.size();
	while (ptr < end)
	{
		int rc = libssh2_sftp_write(handle, ptr, end - ptr);
		if (rc >= 0)
			ptr += rc;
		else
			throw("Failed to write data over SFTP!");
	}

	libssh2_sftp_close(handle);
}













