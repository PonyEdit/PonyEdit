#include "remotechannel.h"
#include "remoteconnection.h"
#include "remoterequest.h"
#include "main/tools.h"
#include "main/dialogwrapper.h"
#include "ssh/connectionstatuswidget.h"
#include "main/global.h"
#include <QThread>
#include <QDebug>
#include "requeststatuswidget.h"
#include "main/dialogwrapper.h"
#include <QMessageBox>

//	Really small skeleton class; actual work done in RemoteChannel and its subclasses
class RemoteChannelThread : public QThread
{
public:
	RemoteChannelThread(RemoteChannel* channel) : QThread(0), mChannel(channel) {}
	virtual void run() { mChannel->threadRun(); }
	RemoteChannel* mChannel;
};

/*********************************/

RemoteChannel::RemoteChannel(RemoteConnection* connection, Type type)
{
	mConnectionId = -1;
	mConnection = connection;
	mRawHandle = NULL;
	mType = type;

	//	Must register the channel before setting the status; otherwise it upsets RemoteConnection.
	connection->registerNewChannel(this);
	setStatus(Opening);
}

void RemoteChannel::setStatus(Status newStatus)
{
	mStatus = newStatus;
	mStatusWaiter.wakeAll();
	mConnection->channelStateChanged(this);
}

void RemoteChannel::sendRequest(RemoteRequest* request)
{
	if (isAcceptingRequests())
	{
		LOCK_MUTEX(mRequestQueueLock);
		mRequestQueue.append(request);
		UNLOCK_MUTEX(mRequestQueueLock);
		mRequestQueueWaiter.wakeAll();
	}
	else
	{
		throw(QObject::tr("Failed to send request: Not connected"));
	}
}

void RemoteChannel::killThread()
{
	//	Slaughter my thread.
	if (mThread)
	{
		mRequestQueueWaiter.wakeAll();

		//	Give threads 2 seconds to comply....
		if (!mThread->wait(2000))
		{
			//	... then just shoot them.
			mThread->terminate();
			mThread->wait();
		}

		delete mThread;
		mThread = NULL;
	}
}

void RemoteChannel::threadRun()
{
	//	Outer loop: connects and tries to pump the queue
	while (!mConnection->isDeliberatelyDisconnecting())
	{
		//	Wait for the connection to succeed / fail at connecting
		bool connectionOk = mConnection->waitUntilOpen(false);
		if (!connectionOk)
			break;

		setStatus(Opening);
		mConnectionId = mConnection->getConnectionId();
		try
		{
			threadConnect();
			setStatus(Open);
		}
		catch(QString err)
		{
			setErrorStatus(err);
			break;
		}

		if (mConnection->isDeliberatelyDisconnecting()) break;
		//	TODO: Handle connection failure

		//	Inner loop: sleeps while there's nothing to do, pumps a message queue when there is.
		while (isAcceptingRequests())
		{
			QList<RemoteRequest*> sendingMessages;
			try
			{
				//	Sleep until there are messages
				if (!mRequestQueue.length())
				{
					QMutex mutex;
					mutex.lock();
					mRequestQueueWaiter.wait(&mutex, 1000);

					threadCheckForNotifications();
				}

				//	Check the request queue for messages
				LOCK_MUTEX(mRequestQueueLock);
				sendingMessages = mRequestQueue;
				mRequestQueue.clear();
				UNLOCK_MUTEX(mRequestQueueLock);

				//	If there are messages to send, the subclass will handle them
				if (sendingMessages.length())
					threadSendMessages(sendingMessages);
			}
			catch (QString error)
			{
				setErrorStatus(error);
				mConnection->setDisconnected();

				qDebug() << "Channel error: " << error;

				mRequestQueueLock.lock();
				sendingMessages.append(mRequestQueue);
				mRequestQueue.clear();
				mRequestQueueLock.unlock();

				foreach (RemoteRequest* rq, sendingMessages)
				{
					rq->handleError(QObject::tr("Connection lost: %1").arg(error));
					delete rq;
				}
			}
		}
	}
}

bool RemoteChannel::waitUntilOpen(int connectionId)
{
	if ((mConnectionId < connectionId || mStatus == Opening) && !mConnection->isDeliberatelyDisconnecting())
	{
		if (Tools::isMainThread())
		{
			//	If this is the main UI thread, show a dialog and actively wait.
			DialogWrapper<ConnectionStatusWidget> dialogWrapper(QObject::tr("Creating Remote Channel"), new ConnectionStatusWidget(mConnection, true), false);
			dialogWrapper.exec();
		}
		else
		{
			//	If this is NOT the main UI thread, use mutexes to passively wait
			QMutex mutex;
			mutex.lock();
			while ((mConnectionId < connectionId || mStatus == Opening) && !mConnection->isDeliberatelyDisconnecting())
			{
				mStatusWaiter.wait(&mutex, 1000);
			}
		}
	}

	return (mStatus != Error);
}

void RemoteChannel::startThread()
{
	mThread = new RemoteChannelThread(this);
	mThread->start();
}

void RemoteChannel::setErrorStatus(const QString& error)
{
	mErrorMessage = error;
	setStatus(Error);
}

bool RemoteChannel::isAcceptingRequests()
{
	return (mStatus != Error) && !mConnection->isDeliberatelyDisconnecting();
}

RequestStatusWidget::Result RemoteChannel::waitForRequest(RemoteRequest* request, const QString& description, bool allowSudo)
{
	DialogWrapper<RequestStatusWidget> dlg(QObject::tr("Please wait..."),
		new RequestStatusWidget(this, request, description, allowSudo), true);
	return static_cast<RequestStatusWidget::Result>(dlg.exec());
}

