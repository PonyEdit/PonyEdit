#include "sshconnection.h"

#include <windows.h>
#include <QDebug>
#include <QTime>

void SshConnection::initializeLib()
{
	//	Initialize winsock
	WSADATA data;
	if (WSAStartup(0x22, &data) != 0)
		throw("Failed to initialize WinSock!");

	//	Initialize libssh2
	if (libssh2_init(0) != 0)
		throw("Failed to initialize SSH library!");
}

SshConnection::SshConnection()
	: mSocket(-1), mSession(0), mChannel(0)
{}

void SshConnection::connect(const char* host, int port)
{
	//	nslookup the hostname
	struct hostent* server = gethostbyname(host);
	if (server == NULL)
		throw("Failed to find host");

	//	Create a socket & connect
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < -1)
		throw("Failed to create a socket!");

	//	Cycle through all the IP addresses returned by the nslookup, try to connect to each
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	int tryAddress = 0;
	while (mSocket < 0 && server->h_addr_list[tryAddress] != 0)
	{
		sin.sin_addr.s_addr = *(u_long*)server->h_addr_list[tryAddress];
		if (::connect(sock, (struct sockaddr*)&sin, sizeof(struct sockaddr_in)) == 0)
			mSocket = sock;
		tryAddress++;
	}
	if (mSocket < 0)
		throw("Failed to connect to host");

	//	Create an SSH2 session
	mSession = libssh2_session_init();
	if (libssh2_session_startup(mSession, mSocket))
	{
		disconnect();
		throw("Failed to set up SSH session");
	}

	//	Fetch the remote host's fingerprint
	mServerFingerprint = QByteArray(libssh2_hostkey_hash(mSession, LIBSSH2_HOSTKEY_HASH_SHA1), 20);
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

	return methods;
}

bool SshConnection::authenticatePassword(const char* username, const char* password)
{
	if (libssh2_userauth_password(mSession, username, password))
		return false;

	createChannel();
	return true;
}

void SshConnection::createChannel()
{
	//	Create a channel
	mChannel = libssh2_channel_open_session(mSession);
	if (!mChannel)
		throw("Failed to create SSH channel!");

	if (libssh2_channel_request_pty(mChannel, "vanilla"))
		throw("Failed to set vanilla pty mode");

	//	Start up a shell
	if (libssh2_channel_shell(mChannel))
		throw("Failed to create a shell");

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
			qDebug() << QByteArray(mTmpBuffer, rc);
			mReadBuffer.append(mTmpBuffer, rc);
		}
		else if (rc < 0 && rc != LIBSSH2_ERROR_EAGAIN)
		{
			qDebug() << "Error code: " << rc;
			throw("Failed to receive from remote host!");
		}
	}
}

QByteArray SshConnection::execute(const char* command)
{
	libssh2_channel_write(mChannel, command, strlen(command));
	return readToPrompt();
}

void SshConnection::writeData(const char* data, int length)
{
	libssh2_channel_write(mChannel, data, length);
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
		int err = libssh2_session_last_error(mSession, &errmsg, &errlen, 0);
		qDebug() << err;
		qDebug() << errmsg;
		qDebug() << errlen;
		throw(errmsg);
	}

	if (libssh2_channel_write(tmpChannel, data, length) < 0)
		throw("Error sending file!");

	libssh2_channel_send_eof(tmpChannel);

	libssh2_channel_wait_closed(tmpChannel);
	libssh2_channel_free(tmpChannel);
}

QByteArray SshConnection::readFile(const char* filename)
{
	struct stat fileInfo;
	LIBSSH2_CHANNEL* tmpChannel = libssh2_scp_recv(mSession, filename, &fileInfo);
	if (!tmpChannel)
		throw("Failed to open remote file!");

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
			throw("Failed to receive file content!");
	}

	return fileContent;
}



















