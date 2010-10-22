#include "sshhost.h"
#include "serverconfigdlg.h"
#include <QMessageBox>

SshHost* SshHost::getHost(const QString& hostName, const QString& userName)
{
	//	TODO: Search through the list of known servers to find the one we want

	//	No known hosts found. Try to create a new one
	bool createdHost = true;
	SshHost* host = createHost(hostName, userName);
	if (!host)
		return NULL;

	//	Now try to connect to the found/created host if not already connected
	if (!host->isConnected())
	{
		if (!host->connect())
		{
			if (createdHost)
				delete host;
			return NULL;
		}
	}

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


SshHost::SshHost(const QString& hostName, const QString& userName)
{
	mConnection = NULL;
	mController = NULL;
	mHostName = hostName;
	mUserName = userName;
	mPort = 22;
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


