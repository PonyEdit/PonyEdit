#ifndef SSHHOST_H
#define SSHHOST_H

#include <QString>

class SshHost
{
public:
	static SshHost* getHost(const QString& hostName, const QString& userName);

	const QString& getHostName() const;
	const QString& getUserName() const;

private:
	SshHost(const QString& hostName, const QString& userName);

	QString mHostName;
	QString mUserName;
	bool mSave;

	static QList<SshHost*> mKnownHosts;
};

#endif // SSHHOST_H
