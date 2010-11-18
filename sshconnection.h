#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include <QByteArray>
#include <libssh2.h>

#define SSH_BUFFER_SIZE 4096
#define SSH_PROMPT "%-remoted-%"

class ISshConnectionCallback
{
public:
	virtual void fileOpenProgress(int percent) {}
};

class SshConnection
{
public:
	enum AuthMethods
	{
		Password = 0x01,
		KeyboardInteractive = 0x02,
		PublicKey = 0x03
	};

public:
	SshConnection();
	~SshConnection();

	void connect(const char* host, int port);
	void disconnect();
	AuthMethods getAuthenticationMethods(const char* username);
	bool authenticatePassword(const char* username, const char* password);
	bool authenticateAgent(const char* username);
	void startRemoteSlave(const char* filename);

	void writeFile(const char* remoteFilename, const char* data, int length);
	QByteArray readFile(const char* filename, ISshConnectionCallback* callback);

	QByteArray execute(const char* command);
	QByteArray readToPrompt();
	QByteArray readLine();
	QByteArray readUntil(const char* marker);
	void writeData(const char* data, int length);
	void sendEof();

	static void initializeLib();

private:
	void createChannel();
	QString getLastError(int rc);

	int mSocket;
	LIBSSH2_SESSION* mSession;
	LIBSSH2_CHANNEL* mChannel;
	QByteArray mServerFingerprint;
	char mTmpBuffer[SSH_BUFFER_SIZE];
	QByteArray mReadBuffer;
};

#endif // SSHCONNECTION_H
