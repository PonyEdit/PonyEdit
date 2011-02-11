#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include <QByteArray>
#include <libssh2.h>
#include <QMap>
#include <QMutex>

#define SSH_BUFFER_SIZE 4096
#define SSH_PROMPT "%-ponyedit-%"

class ISshConnectionCallback
{
public:
        virtual void fileOpenProgress(int /*percent*/) {}
};

class RawSshConnection
{
public:
	enum AuthMethods
	{
		Password = 0x01,
		KeyboardInteractive = 0x02,
		PublicKey = 0x03
	};

	struct Channel
	{
		LIBSSH2_CHANNEL* handle;
		char tmpBuffer[SSH_BUFFER_SIZE];
		QByteArray readBuffer;
	};

public:
	RawSshConnection();
	~RawSshConnection();

	void connect(const char* host, int port);
	void disconnect();
	AuthMethods getAuthenticationMethods(const char* username);
	bool authenticatePassword(const char* username, const char* password);
	bool authenticateKeyFile(const char* filename, const char* username, const char* password, bool* keyRejected);
	bool authenticateAgent(const char* username);
	void startRemoteSlave();

	void writeFile(const char* remoteFilename, const char* data, int length);
	QByteArray readFile(const char* filename, ISshConnectionCallback* callback);

	QByteArray execute(Channel* channel, const char* command);
	QByteArray readToPrompt(Channel* channel);
	QByteArray readLine(Channel* channel);
	QByteArray readUntil(Channel* channel, const char* marker);
	void writeData(Channel* channel, const char* data, int length);
	void sendEof(Channel* channel);

	static void initializeLib();

	inline const QByteArray& getServerFingerprint() { return mServerFingerprint; }
	inline static QByteArray getExpectedFingerprint(const QString& host) { return sKnownHostKeys.value(host); }
	static void saveFingerprint(const QString& host, const QByteArray& fingerprint);

	Channel* createShellChannel();

private:
	QString getLastError(int rc = -1);
	static void saveKnownHostKeys();

	int mSocket;
	LIBSSH2_SESSION* mSession;
	QByteArray mServerFingerprint;

	QMutex mAccessMutex;

	static QMap<QString, QByteArray> sKnownHostKeys;
};

#endif // SSHCONNECTION_H
