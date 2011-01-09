#include "remoteconnection.h"

RemoteConnection::RemoteConnection()
{
	mDeliberatelyDisconnecting = false;
	mStatus = Uninitialized;
	mInputDialog = NULL;
}

QString RemoteConnection::getStatusString()
{
	switch (getBaseStatus())
	{
	case Uninitialized:
		return tr("Initializing ...");

	case Connecting:
		return tr("Connecting ...");

	case Authenticating:
		return tr("Authenticating ...");

	case Configuring:
		return tr("Configuring ...");

	case Connected:
		return tr("Connected");

	case Disconnecting:
		return tr("Disconnecting ...");

	case Disconnected:
		return tr("Disconnected");

	case Error:
		return tr("Error");

	default:
		return tr("Unknown");
	}
}

QPixmap RemoteConnection::getStatusIcon()
{
	switch (getBaseStatus())
	{
	case Uninitialized:
	case Disconnected:
		return QPixmap(":/icons/disconnected.png");

	case Connecting:
	case Authenticating:
	case Configuring:
	case Disconnecting:
		return QPixmap(":/icons/loading.png");

	case Connected:
		return QPixmap(":/icons/server.png");

	case Error:
	default:
		return QPixmap(":/icons/error.png");
	}
}

void RemoteConnection::setStatus(Status newStatus)
{
	mStatus = newStatus;
	emit statusChanged();
}

void RemoteConnection::setErrorStatus(const QString &errorMessage)
{
	mErrorMessage = errorMessage;
	setStatus(Error);
}

void RemoteConnection::waitForInput(DialogFunction dialogFunction, DialogCallback callbackFunction)
{
	mInputDialog = dialogFunction;
	mDialogCallback = callbackFunction;

	QMutex mutex;
	mutex.lock();
	setStatus(static_cast<Status>(WaitingOnInput | mStatus));
	mInputDialogWait.wait(&mutex);

	mInputDialog = NULL;
	mDialogCallback = NULL;
}

void RemoteConnection::disconnect(bool deliberate)
{
	if (deliberate) mDeliberatelyDisconnecting = true;
	setStatus(Disconnecting);
}

bool RemoteConnection::isDisconnecting()
{
	Status baseStatus = getBaseStatus();
	return (baseStatus == Disconnecting || baseStatus == Disconnected);
}






