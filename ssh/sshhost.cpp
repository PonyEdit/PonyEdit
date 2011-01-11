#include "ssh/sshhost.h"
#include "ssh/serverconfigdlg.h"
#include "main/tools.h"
#include <QMessageBox>
#include "main/globaldispatcher.h"
#include <QDebug>
#include "connectionstatuswidget.h"
#include "main/dialogwrapper.h"

QList<SshHost*> SshHost::sKnownHosts;

SshHost* SshHost::getHost(const QString& hostName, const QString& userName, bool save)
{
	SshHost* host = NULL;
	bool createdHost = false;

	//	TODO: Search through the list of known servers to find the one we want
	foreach (SshHost* seekHost, sKnownHosts)
	{
		if (hostName == seekHost->mHostName && (userName.isEmpty() || userName == seekHost->mUserName))
		{
			host = seekHost;
			break;
		}
	}

	if (!host)
	{
		//	No known hosts found. Try to create a new one
		createdHost = true;
		host = createHost(hostName, userName, save);
		if (!host)
			return NULL;
	}

	//	Add to the list of known hosts, and save the list. If this one is not flagged to be saved, saveServers will go past it.
	if (createdHost)
		sKnownHosts.append(host);
	Tools::saveServers();

	gDispatcher->emitSshServersUpdated();

	return host;
}

SshHost* SshHost::getBlankHost(bool save)
{
	SshHost* host = new SshHost("", "");
	if(!host)
		return NULL;

	host->setSave(save);

	sKnownHosts.append(host);

	gDispatcher->emitSshServersUpdated();

	return host;
}

SshHost* SshHost::createHost(const QString& hostName, const QString& userName, bool save)
{
	SshHost* newHost = new SshHost(hostName, userName);

	if(save)
		newHost->setSave(true);

	ServerConfigDlg serverConfigDlg;
	serverConfigDlg.setEditHost(newHost);
	bool accepted = serverConfigDlg.exec();

	if (!accepted)
	{
		delete newHost;
		return NULL;
	}

	return newHost;
}

SshHost::SshHost()
{
	mController = NULL;
	mSave = true;
	mSavePassword = false;
	mPort = 22;
}

SshHost::SshHost(const QString& hostName, const QString& userName)
{
	mController = NULL;
	mHostName = hostName;
	mUserName = userName;
	mPort = 22;
	mDefaultDirectory = "~";
	mSavePassword = false;
	mSave = false;
}

SshHost::~SshHost()
{
	disconnect();
	sKnownHosts.removeAll(this);
}

bool SshHost::isConnected() const
{
	return (mController != NULL && mController->getStatus() == SshRemoteController::Connected);
}

bool SshHost::connect()
{
	disconnect();

	//	Show a connection dialog; it will manage the whole connection process
	mController = new SshRemoteController(this);
	ConnectionStatusWidget* statusWidget = new ConnectionStatusWidget(mController, true);
	DialogWrapper<ConnectionStatusWidget> dialogWrapper(statusWidget);

	return (dialogWrapper.exec() == QDialog::Accepted);
}

void SshHost::disconnect()
{
	if (mController != NULL)
	{
		delete mController;
		mController = NULL;
	}
}

bool SshHost::ensureConnection()
{
	return isConnected() || connect();
}

void SshHost::recordKnownHost(SshHost* host)
{
	sKnownHosts.append(host);
}

QString SshHost::getDefaultPath()
{
	return (mUserName.isEmpty() ? "" : mUserName + "@") + mHostName + ":" + mDefaultDirectory;
}

Location SshHost::getDefaultLocation()
{
	return Location(getDefaultPath());
}

void SshHost::registerOpenFile(SshFile* file)
{
	mOpenFiles.append(file);
}

void SshHost::unregisterOpenFile(SshFile* file)
{
	mOpenFiles.removeOne(file);
}

int SshHost::numOpenFiles() const
{
	return mOpenFiles.count();
}

const QList<SshFile*> SshHost::getOpenFiles() const
{
	return mOpenFiles;
}



