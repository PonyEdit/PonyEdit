#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include <QByteArray>
#include <libssh2.h>
#include <libssh2_sftp.h>
#include <QMap>
#include <QMutex>
#include "file/location.h"

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
		PublicKey = 0x04
	};

	struct Channel
	{
		LIBSSH2_CHANNEL* handle;
		LIBSSH2_SFTP* sftpSession;
		char tmpBuffer[SSH_BUFFER_SIZE];
		QByteArray readBuffer;
	};

public:
	RawSshConnection();
	~RawSshConnection();

	void connect(const char* host, int port);
	void disconnect();
	AuthMethods getAuthenticationMethods(const char* username);
	bool authenticatePassword(const char* username, const char* password, AuthMethods authMethods);
	bool authenticateKeyFile(const char* filename, const char* username, const char* password, bool* keyRejected);
	bool authenticateAgent(const char* username);
	void startRemoteSlave();

	void writeFile(const char* remoteFilename, const char* data, int length);
	QByteArray readFile(const char* filename, ISshConnectionCallback* callback);

	QByteArray execute(Channel* channel, const char* command);
	QByteArray readToPrompt(Channel* channel);
	QByteArray readLine(Channel* channel);
	QByteArray readUntil(Channel* channel, const char* marker);
	bool pollData(Channel* channel);
	void writeData(Channel* channel, const char* data, int length);
	void sendEof(Channel* channel);

	QList<Location> getFTPListing(Channel* channel, const Location& parent, bool includeHidden);
	QByteArray readFTPFile(Channel* channel, const Location& location, ISshConnectionCallback* callback);
	void writeFTPFile(Channel* channel, const Location& location, const QByteArray& content);

	static void initializeLib();
	static void cleanup();

	inline const QByteArray& getServerFingerprint() { return mServerFingerprint; }
	inline static QByteArray getExpectedFingerprint(const QString& host) { return sKnownHostKeys.value(host); }
	static void saveFingerprint(const QString& host, const QByteArray& fingerprint);

	Channel* createShellChannel();
	Channel* createFTPChannel();

private:
	QString getLastError(int rc = -1);
	static void saveKnownHostKeys();
	QString tidyFtpPath(const QString& path);

	static void interactiveAuthCallback(const char*, int, const char*, int, int,
		const LIBSSH2_USERAUTH_KBDINT_PROMPT*, LIBSSH2_USERAUTH_KBDINT_RESPONSE*, void**);

	int mSocket;
	LIBSSH2_SESSION* mSession;
	QByteArray mServerFingerprint;
	const char* mKeyboardAuthPassword;

	QMutex mAccessMutex;

	static QMap<QString, QByteArray> sKnownHostKeys;
};

#endif // SSHCONNECTION_H
