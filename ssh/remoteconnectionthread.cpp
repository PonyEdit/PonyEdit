#include "remoteconnectionthread.h"
#include "remoteconnection.h"
#include <QDebug>

RemoteConnectionThread::RemoteConnectionThread(RemoteConnection* connection) :
	QThread(0)
{
	mConnection = connection;
}

bool RemoteConnectionThread::connect()
{
	qDebug() << "Attempting to connect...";

	mConnection->setStatus(RemoteConnection::Connecting);
	mConnection->mConnectionId++;

	if (mConnection->threadConnect())
	{
		mConnection->setStatus(RemoteConnection::OpeningChannels);

		if (mConnection->mOpenChannels.length() == 0)
		{
			//	Always open Primary channel on first successful connection
			mConnection->threadOpenPrimaryChannel();
		}

		//	Wait for all of my channels to reconnect (or fail to) before changing state to "Connected"
		foreach (RemoteChannel* channel, mConnection->mOpenChannels)
			channel->waitUntilOpen(mConnection->mConnectionId);
	}
	else
		return false;

	return true;
}

void RemoteConnectionThread::run()
{
	while (!mConnection->isDeliberatelyDisconnecting())
	{
		//	If in an error state, and there is no reason to continue living, just give up.
		if (mConnection->getStatus() & RemoteConnection::Error && !mConnection->hasReasonToLive())
			break;

		if (!mConnection->isConnected())
			connect();

		QMutex mutex;
		mutex.lock();
		mSleeper.wait(&mutex, 5000);
	}

	//	Make sure all attached channel threads die before this one does.
	foreach (RemoteChannel* channel, mConnection->mOpenChannels)
		channel->killThread();
}

void RemoteConnectionThread::wake()
{
	mSleeper.wakeAll();
}
















