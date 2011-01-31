#include "remoteconnection.h"
#include "remoteconnectionthread.h"
#include "slavechannel.h"
#include <QDebug>

RemoteConnection::RemoteConnection()
{
	mStatus = Uninitialized;
	mDeliberatelyDisconnecting = false;
	mConnectionId = 0;
}

void RemoteConnection::startConnectionThread()
{
	mThread = new RemoteConnectionThread(this);
	mThread->start();
}

void RemoteConnection::registerNewChannel(RemoteChannel* channel)
{
	mOpenChannels.append(channel);
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

	case Connected:
		return tr("Connected");

	case OpeningChannels:
		return tr("Configuring ...");

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
	case Disconnecting:
	case OpeningChannels:
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
	mThread->wake();
	mStatusWaiter.wakeAll();
}

void RemoteConnection::setErrorStatus(const QString &errorMessage)
{
	mErrorMessage = errorMessage;
	setStatus(Error);
}

void RemoteConnection::waitForInput(DialogFunction dialogFunction, DialogCallback callbackFunction, QVariant param)
{
	mInputDialog = dialogFunction;
	mDialogCallback = callbackFunction;
	mDialogParameter = param;

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

RemoteChannel* RemoteConnection::getChannel(RemoteChannel::Type type)
{
	foreach (RemoteChannel* channel, mOpenChannels)
		if (channel->getType() == type)
			return channel;

	//	Not found. Try to open one
	return openChannel(type);
}

bool RemoteConnection::waitUntilOpen()
{
	qDebug() << "Entering RemoteConnection::waitUntilOpen";

	QMutex mutex;
	mutex.lock();

	while (1)
	{
		Status baseStatus = getBaseStatus();
		if (baseStatus == OpeningChannels || baseStatus == Connected || baseStatus == Error)
			break;

		mStatusWaiter.wait(&mutex, 1000);
	}

	qDebug() << "Exiting RemoteConnection::waitUntilOpen";

	return (getBaseStatus() != Error);
}

SlaveChannel* RemoteConnection::getSlaveChannel()
{
	return static_cast<SlaveChannel*>(getChannel(RemoteChannel::Slave));
}

SlaveChannel* RemoteConnection::getSudoChannel()
{
	return static_cast<SlaveChannel*>(getChannel(RemoteChannel::SudoSlave));
}





