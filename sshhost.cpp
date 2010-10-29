#include "sshhost.h"
#include "serverconfigdlg.h"
#include "tools.h"
#include <QMessageBox>
#include "globaldispatcher.h"
#include <QDebug>

QList<SshHost*> SshHost::sKnownHosts;

SshHost* SshHost::getHost(const QString& hostName, const QString& userName)
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
		host = createHost(hostName, userName);
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

SshHost* SshHost::createHost(const QString& hostName, const QString& userName)
{
	SshHost* newHost = new SshHost(hostName, userName);

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
	mConnection = NULL;
	mController = NULL;
	mSave = true;
	mSavePassword = false;
	mPort = 22;
}

SshHost::SshHost(const QString& hostName, const QString& userName)
{
	mConnection = NULL;
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
}

bool SshHost::isConnected() const
{
	return (mConnection != NULL);
}

bool SshHost::connect()
{
	disconnect();

	//	Establish a raw SSH connection
	mConnection = new SshConnection();
	try
	{
		mConnection->connect(mHostName.toUtf8(), mPort);
		mConnection->authenticatePassword(mUserName.toUtf8(), mPassword.toUtf8());
	}
	catch (const char* error)
	{
		QMessageBox::critical(NULL, this->mHostName + ": Failed to connnect", error);
		return false;
	}

	//	Attach a controller to the connection. TODO: More error handling.
	mController = new SshRemoteController();
	mController->attach(mConnection);
	mController->splitThread();

	return true;
}

void SshHost::disconnect()
{
	if (mController != NULL)
	{
		delete mController;
		mController = NULL;
	}

	if (mConnection != NULL)
	{
		delete mConnection;
		mConnection = NULL;
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


