#include "sshhost.h"
#include "serverconfigdlg.h"

SshHost* SshHost::getHost(const QString& hostName, const QString& userName)
{
	//	TODO: Search through the list of known servers to find the one we want

	//	No known hosts found. Create a new host and immediately throw up a config dialog for it.
	SshHost* newHost = new SshHost(hostName, userName);

	ServerConfigDlg serverConfigDlg;
	serverConfigDlg.setEditHost(newHost);
	serverConfigDlg.exec();

	return newHost;
}


SshHost::SshHost(const QString& hostName, const QString& userName)
{
	mHostName = hostName;
	mUserName = userName;
}

const QString& SshHost::getHostName() const
{
	return mHostName;
}

const QString& SshHost::getUserName() const
{
	return mUserName;
}
