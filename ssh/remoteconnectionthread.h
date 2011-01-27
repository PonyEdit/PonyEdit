#ifndef REMOTECONNECTIONTHREAD_H
#define REMOTECONNECTIONTHREAD_H

#include <QWaitCondition>
#include <QThread>
#include <QMutex>

class RemoteChannel;
class RemoteConnection;
class RemoteConnectionThread : public QThread
{
    Q_OBJECT

public:
	explicit RemoteConnectionThread(RemoteConnection* connection);

	void wake();

protected:
	void connect();
	void run();

	RemoteConnection* mConnection;
	QWaitCondition mSleeper;
};

#endif // REMOTECONNECTIONTHREAD_H
