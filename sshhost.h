#ifndef SSHHOST_H
#define SSHHOST_H

#include <QString>
#include "sshconnection.h"
#include "sshremotecontroller.h"

class SshRemoteController;
class SshHost
{
public:
	SshHost() {}	//	Only use the default constructor at load-time. Otherwise notifications of new SshHosts won't be sent to UI.
	static SshHost* getHost(const QString& hostName = QString(), const QString& userName = QString());
	static SshHost* createHost(const QString& hostName = QString(), const QString& userName = QString());
	static QList<SshHost*>& getKnownHosts() { return sKnownHosts; }
	~SshHost();

	static void recordKnownHost(SshHost* host);	//	Only use at load-time.

	inline const QString& getHostName() const { return mHostName; }
	inline const QString& getUserName() const { return mUserName; }
	inline const QString& getPassword() const { return mPassword; }
	inline const QString& getDefaultDirectory() const { return mDefaultDirectory; }
	inline int getPort() const { return mPort; }
	inline bool getSave() const { return mSave; }
	inline const QString& getName() const { return mName; }
	inline bool getSavePassword() const { return mSavePassword; }

	inline void setHostName(const QString& hostName) { mHostName = hostName; }
	inline void setUserName(const QString& userName) { mUserName = userName; }
	inline void setPassword(const QString& password) { mPassword = password; }
	inline void setDefaultDirectory(const QString& defaultDirectory) { mDefaultDirectory = defaultDirectory; }
	inline void setPort(int port) { mPort = port; }
	inline void setSave(bool save) { mSave = save; }
	inline void setName(const QString& name) { mName = name; }
	inline void setSavePassword(bool savePassword) { mSavePassword = savePassword; }

	bool isConnected() const;
	bool ensureConnection();
	bool connect();
	void disconnect();

	QString getDefaultPath();
	Location getDefaultLocation();

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

	QString mDefaultDirectory;

	QString mName;
	bool mSave;

	static QList<SshHost*> sKnownHosts;
};

#endif // SSHHOST_H
