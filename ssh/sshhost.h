#ifndef SSHHOST_H
#define SSHHOST_H

#include <QString>

#include "file/location.h"
#include "ssh/sshconnection.h"

class SlaveFile;
class SshHost
{
public:
	SshHost(); 	//	Only use the default constructor at load-time. Otherwise notifications of new SshHosts won't be sent to UI.
	static SshHost* getHost(const QString& hostName = QString(), const QString& userName = QString(), bool save = false);
	static SshHost* getBlankHost(bool save = false);
	static SshHost* createHost(const QString& hostName = QString(), const QString& userName = QString(), bool save = false);
	static QList<SshHost*>& getKnownHosts() { return sKnownHosts; }
	static void cleanupHosts();
	~SshHost();

	static void recordKnownHost(SshHost* host);	//	Only use at load-time.

	inline const QString& getHostName() const { return mHostName; }
	inline const QString& getUserName() const { return mUserName; }
	inline const QString& getPassword() const { return mPassword; }
	inline const QString& getKeyPassphrase() const { return mKeyPassphrase; }
	inline const QString& getKeyFile() const { return mKeyFile; }
	inline const QString& getDefaultDirectory() const { return mDefaultDirectory; }
	inline int getPort() const { return mPort; }
	inline bool getSave() const { return mSave; }
	inline const QString& getName() const { return mName; }
	inline bool getSavePassword() const { return mSavePassword; }
	inline bool getSaveKeyPassphrase() const { return mSaveKeyPassphrase; }
	inline const QString& getSudoPassword() const { return mSudoPassword; }

	inline void setHostName(const QString& hostName) { mHostName = hostName; }
	inline void setUserName(const QString& userName) { mUserName = userName; }
	inline void setPassword(const QString& password) { mPassword = password; }
	inline void setKeyPassphrase(const QString& keyPassphrase) { mKeyPassphrase = keyPassphrase; }
	inline void setKeyFile(const QString& keyFile) { mKeyFile = keyFile; }
	inline void setDefaultDirectory(const QString& defaultDirectory) { mDefaultDirectory = defaultDirectory; }
	inline void setPort(int port) { mPort = port; }
	inline void setSave(bool save) { mSave = save; }
	inline void setName(const QString& name) { mName = name; }
	inline void setSavePassword(bool savePassword) { mSavePassword = savePassword; }
	inline void setSaveKeyPassphrase(bool saveKeyPassphrase) { mSaveKeyPassphrase = saveKeyPassphrase; }
	inline void setSudoPassword(const QString& sudoPassword) { mSudoPassword = sudoPassword; }

	QString getDefaultPath();
	Location getDefaultLocation();

	void registerOpenFile(SlaveFile* file);
	void unregisterOpenFile(SlaveFile* file);
	int numOpenFiles() const;
	const QList<SlaveFile*> getOpenFiles() const;

	//	Connection stuff
	SshConnection* getConnection();		//	*NOT RE-ENTRANT* If already connected, this returns the connection. Otherwise, this connects!!

private:
	SshHost(const QString& hostName, const QString& userName);

	SshConnection* mConnection;
	QList<SlaveFile*> mOpenFiles;

	QString mHostName;
	int mPort;

	QString mUserName;
	QString mPassword;
	QString mKeyFile;
	QString mKeyPassphrase;
	bool mSavePassword;
	bool mSaveKeyPassphrase;

	QString mSudoPassword;

	QString mDefaultDirectory;

	QString mName;
	bool mSave;

	static QList<SshHost*> sKnownHosts;
};

#endif // SSHHOST_H
