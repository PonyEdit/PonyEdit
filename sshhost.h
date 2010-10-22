#ifndef SSHHOST_H
#define SSHHOST_H

#include <QString>

class SshHost
{
public:
	static SshHost* getHost(const QString& hostName, const QString& userName);

	inline const QString& getHostName() const { return mHostName; }
	inline const QString& getUserName() const { return mUserName; }
	inline const QString& getPassword() const { return mPassword; }
	inline int getPort() const { return mPort; }

	inline void setHostName(const QString& hostName) { mHostName = hostName; }
	inline void setUserName(const QString& userName) { mUserName = userName; }
	inline void setPassword(const QString& password) { mPassword = password; }
	inline void setPort(int port) { mPort = port; }

private:
	SshHost(const QString& hostName, const QString& userName);

	QString mHostName;
	int mPort;

	QString mUserName;
	QString mPassword;
	bool mSavePassword;

	QString mName;
	bool mSave;

	static QList<SshHost*> mKnownHosts;
};

#endif // SSHHOST_H
