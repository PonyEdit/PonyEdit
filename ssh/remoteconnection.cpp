#include "remoteconnection.h"
#include "remoteconnectionthread.h"
#include "slavechannel.h"
#include "main/tools.h"
#include "main/dialogwrapper.h"
#include "ssh/connectionstatuswidget.h"
#include <QDebug>

RemoteConnection::RemoteConnection()
{
	mStatus = Uninitialized;
	mDeliberatelyDisconnecting = false;
	mConnectionId = 0;
	mThread = NULL;

	qDebug() << "New remote connection object at " << (void*)this;
}

RemoteConnection::~RemoteConnection()
{
	killThread();
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
		return tr("Opening Channel ...");

	case Disconnecting:
		return tr("Disconnecting ...");

	case Disconnected:
		return tr("Disconnected");

	case Error:
		return tr("Error - %1").arg(mErrorMessage);

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
	mDeliberatelyDisconnecting = deliberate;
	setStatus(Disconnecting);
}

bool RemoteConnection::isDisconnecting()
{
	Status baseStatus = getBaseStatus();
	return (baseStatus == Disconnecting || baseStatus == Disconnected);
}

RemoteChannel* RemoteConnection::getChannel(RemoteChannel::Type type)
{
	//	See if the channel type is already open
	foreach (RemoteChannel* channel, mOpenChannels)
	{
		if (channel->getType() == type)
		{
			if (channel->waitUntilOpen(mConnectionId))
				return channel;

			return NULL;
		}
	}

	//	If it's not already open, start it opening
	return openChannel(type);
}

bool RemoteConnection::waitUntilOpen(bool waitForChannels)
{
	Status baseStatus = getBaseStatus();
	if (!((baseStatus == OpeningChannels && !waitForChannels) || baseStatus == Connected || baseStatus == Error))
	{
		if (Tools::isMainThread())
		{
			//	If this is the main UI thread, show a dialog and actively wait.
			DialogWrapper<ConnectionStatusWidget> dialogWrapper(tr("Opening Connection"), new ConnectionStatusWidget(this, true), false);
			dialogWrapper.exec();
		}
		else
		{
			//	If this is NOT the main UI thread, use mutexes to passively wait
			QMutex mutex;
			mutex.lock();
			while (1)
			{
				Status baseStatus = getBaseStatus();
				if ((baseStatus == OpeningChannels && !waitForChannels) || baseStatus == Connected || baseStatus == Error)
					break;

				mStatusWaiter.wait(&mutex, 1000);
			}
		}
	}

	return isConnected();
}

SlaveChannel* RemoteConnection::getSlaveChannel()
{
	return static_cast<SlaveChannel*>(getChannel(RemoteChannel::Slave));
}

SlaveChannel* RemoteConnection::getSudoChannel()
{
	return static_cast<SlaveChannel*>(getChannel(RemoteChannel::SudoSlave));
}

void RemoteConnection::recordChannelOpening()
{
	mChannelsOpening++;
	if (mChannelsOpening == 1)
		setBaseStatus(OpeningChannels);
}

void RemoteConnection::recordChannelOpen()
{
	mChannelsOpening--;
	if (mChannelsOpening <= 0)
	{
		mChannelsOpening = 0;
		setBaseStatus(Connected);
	}
}

void RemoteConnection::channelStateChanged(RemoteChannel* /*channel*/)
{
	//	Work out a collective state of my channels
	bool channelOpening = false;
	bool channelError = false;
	QString channelErrorText;
	foreach (RemoteChannel* channel, mOpenChannels)
	{
		if (channel->isOpening())
			channelOpening = true;
		else if (channel->isError())
		{
			channelErrorText = channel->getError();
			channelError = true;
		}
	}

	//	Set my state accordingly
	if (channelError)
		setErrorStatus(channelErrorText);
	else if (channelOpening)
		setBaseStatus(OpeningChannels);
	else
		setBaseStatus(Connected);
}

void RemoteConnection::killThread()
{
	//	Slaughter my thread.
	mDeliberatelyDisconnecting = true;
	if (mThread)
	{
		mThread->wake();

		//	Give threads 5 seconds to comply....
		if (!mThread->wait(5000))
		{
			//	... then just shoot them.
			mThread->terminate();
			mThread->wait();
		}

		delete mThread;
		mThread = NULL;
	}
}





