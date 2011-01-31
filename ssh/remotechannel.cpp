#include "remotechannel.h"
#include "remoteconnection.h"
#include "remoterequest.h"
#include <QThread>
#include <QDebug>

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
	mStatus = Opening;
	mConnection = connection;
	mRawHandle = NULL;
	mType = type;

	connection->registerNewChannel(this);
}

void RemoteChannel::setStatus(Status newStatus)
{
	mStatus = newStatus;
	mStatusWaiter.wakeAll();
}

void RemoteChannel::sendRequest(RemoteRequest* request)
{
	if (isAcceptingRequests())
	{
		mRequestQueueLock.lock();
		mRequestQueue.append(request);
		mRequestQueueLock.unlock();
		mRequestQueueWaiter.wakeAll();
	}
	else
	{
		throw(QObject::tr("Failed to send request: Not connected"));
	}
}

void RemoteChannel::threadRun()
{
	//	Outer loop: connects and tries to pump the queue
	while (!mConnection->isDeliberatelyDisconnecting())
	{
		//	Wait for the connection to succeed / fail at connecting
		bool connectionOk = mConnection->waitUntilOpen();
		if (!connectionOk)
			break;

		setStatus(Opening);
		mConnectionId = mConnection->getConnectionId();
		threadConnect();
		setStatus(Open);

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
				}

				//	Check the request queue for messages
				mRequestQueueLock.lock();
				sendingMessages = mRequestQueue;
				mRequestQueue.clear();
				mRequestQueueLock.unlock();

				//	If there are messages to send, the subclass will handle them
				if (sendingMessages.length())
					threadSendMessages(sendingMessages);
			}
			catch (QString error)
			{
				setStatus(Error);
				mConnection->setDisconnected();

				qDebug() << "Channel error: " << error;

				mRequestQueueLock.lock();
				sendingMessages.append(mRequestQueue);
				mRequestQueue.clear();
				mRequestQueueLock.unlock();

				foreach (RemoteRequest* rq, sendingMessages)
				{
					rq->error(QObject::tr("Connection lost: %1").arg(error));
					delete rq;
				}
			}
		}
	}
}

bool RemoteChannel::waitUntilOpen(int connectionId)
{
	qDebug() << "Entering RemoteChannel::waitUntilOpen";

	QMutex mutex;
	mutex.lock();

	while (mConnectionId < connectionId || mStatus == Opening)
		mStatusWaiter.wait(&mutex, 1000);

	qDebug() << "Exiting RemoteChannel::waitUntilOpen";

	return (mStatus == Open);
}

void RemoteChannel::startThread()
{
	mThread = new RemoteChannelThread(this);
	mThread->start();
}












