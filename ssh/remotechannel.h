#ifndef REMOTECHANNEL_H
#define REMOTECHANNEL_H

#include <QFlags>
#include <QMutex>
#include <QList>
#include <QWaitCondition>
#include <QObject>

class RemoteConnection;
class RawChannelHandle;
class RemoteChannelThread;
class RemoteRequest;

class RemoteChannel
{
public:
	enum Status { Opening, Open, Error };

	enum Type
	{
		Slave        = 0x0001,
		Shell        = 0x0002,
		FTP          = 0x0003,
		BaseTypeMask = 0x0FFF,

		Sudo         = 0x1000,
		SudoSlave    = 0x1001,
	};
	Q_DECLARE_FLAGS(Types, Type);

	RemoteChannel(RemoteConnection* connection, Type type);

	void sendRequest(RemoteRequest* request);
	inline Type getType() const { return mType; }
	void threadRun();	//	Only called from RemoteChannelThread::run

	bool waitUntilOpen(int connectionId);

	inline bool isOpening() { return mStatus == Opening; }
	inline bool isError() { return mStatus == Error; }
	inline const QString& getError() const { return mErrorMessage; }

protected:
	void startThread();	//	Called in subclass constructor
	virtual void threadConnect() = 0;
	virtual void threadSendMessages(QList<RemoteRequest*>& messages) = 0;
	void setStatus(Status newStatus);
	void setErrorStatus(const QString& error);
	inline bool isAcceptingRequests() const { return mStatus != Error; }

	RemoteConnection* mConnection;
	RawChannelHandle* mRawHandle;
	Type mType;
	int mConnectionId;

	RemoteChannelThread* mThread;
	QMutex mRequestQueueLock;
	QList<RemoteRequest*> mRequestQueue;
	QWaitCondition mRequestQueueWaiter;

	QWaitCondition mStatusWaiter;
	Status mStatus;
	QString mErrorMessage;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(RemoteChannel::Types);

#endif // REMOTECHANNEL_H
