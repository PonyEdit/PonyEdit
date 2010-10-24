#ifndef SSHHOST_H
#define SSHHOST_H

#include <QString>
#include "sshconnection.h"
#include "sshremotecontroller.h"

class SshRemoteController;
class SshHost
{
public:
	static SshHost* getHost(const QString& hostName = QString(), const QString& userName = QString());
	static SshHost* createHost(const QString& hostName = QString(), const QString& userName = QString());
	~SshHost();

	inline const QString& getHostName() const { return mHostName; }
	inline const QString& getUserName() const { return mUserName; }
	inline const QString& getPassword() const { return mPassword; }
	inline int getPort() const { return mPort; }

	inline void setHostName(const QString& hostName) { mHostName = hostName; }
	inline void setUserName(const QString& userName) { mUserName = userName; }
	inline void setPassword(const QString& password) { mPassword = password; }
	inline void setPort(int port) { mPort = port; }

	bool isConnected() const;
	bool ensureConnection();
	bool connect();
	void disconnect();

	//	Only usable when connected:
	inline SshRemoteController* getController() { return mController; }
	inline const QString& getHomeDirectory() { return mController->getHomeDirectory(); }

private:
	SshHost(const QString& hostName, const QString& userName);

	SshConnection* mConnection;
	SshRemoteController* mController;

	QString mHostName;
	int mPort;

	QString mUserName;
	QString mPassword;
	bool mSavePassword;

	QString mName;
	bool mSave;

	static QList<SshHost*> sKnownHosts;
};

#endif // SSHHOST_H
