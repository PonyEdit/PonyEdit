#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include <QObject>
#include <QPixmap>
#include <QWidget>
#include <QDialogButtonBox>
#include <QWaitCondition>
#include <QMutex>
#include <QFlags>
#include <QVariant>
#include "remotechannel.h"

class RemoteConnection;
class ConnectionStatusWidget;
class RemoteConnectionThread;
class SlaveChannel;

typedef void(*DialogFunction)(ConnectionStatusWidget*, RemoteConnection*, QWidget*, QVariant);
typedef bool(*DialogCallback)(ConnectionStatusWidget*, RemoteConnection*, QDialogButtonBox::ButtonRole, QVariant);

class RawChannelHandle {};

class RemoteConnection : public QObject
{
	friend class RemoteConnectionThread;
	Q_OBJECT

public:
	enum Status
	{
		Uninitialized   = 0x0000,
		Connecting      = 0x0001,
		Authenticating  = 0x0002,
		OpeningChannels = 0x0004,
		Connected       = 0x0008,
		Disconnecting   = 0x0010,
		Disconnected    = 0x0011,
		Error           = 0x0012,
		BaseStatusMask  = 0x0FFF,

		WaitingOnInput  = 0x1000
	};

public:
	RemoteConnection();

	virtual RemoteChannel* getChannel(RemoteChannel::Type type);
	SlaveChannel* getSlaveChannel();
	void registerNewChannel(RemoteChannel* channel);	//	Only call from RemoteChannel constructor
	virtual RawChannelHandle* createRawSlaveChannel() { throw(tr("Invalid operation")); }
	virtual void sendLine(RawChannelHandle* /*handle*/, const QByteArray& /*data*/) { throw(tr("Invalid operation")); }
	virtual QByteArray readLine(RawChannelHandle* /*handle*/) { throw(tr("Invalid operation")); }

	inline void setHomeDirectory(const QString& homeDirectory) { mHomeDirectory = homeDirectory; }
	inline const QString& getHomeDirectory() { return mHomeDirectory; }

	inline Status getStatus() { return mStatus; }
	inline Status getBaseStatus() { return static_cast<Status>(mStatus & BaseStatusMask); }
	QString getStatusString();
	QPixmap getStatusIcon();

	inline bool isDeliberatelyDisconnecting() { return mDeliberatelyDisconnecting; }
	inline bool isConnected() { return mStatus & Connected; }
	bool isDisconnecting();
	inline bool isWaitingOnInput() { return mStatus & WaitingOnInput; }

	void disconnect(bool deliberate);
	void setErrorStatus(const QString& errorMessage);
	void setDisconnected() { setStatus(Disconnected); }

	virtual QString getName() = 0;

	void waitForInput(DialogFunction dialogFunction, DialogCallback callbackFunction = DefaultDialogCallback, QVariant param = QVariant());
	inline void populateInputDialog(ConnectionStatusWidget* statusWidget, QWidget* target) { mInputDialog(statusWidget, this, target, mDialogParameter); }
	inline bool inputDialogCallback(ConnectionStatusWidget* statusWidget, QDialogButtonBox::ButtonRole buttonRole) { return mDialogCallback(statusWidget, this, buttonRole, mDialogParameter); }
	static bool DefaultDialogCallback(ConnectionStatusWidget*, RemoteConnection*, QDialogButtonBox::ButtonRole, QVariant) { return true; }
	inline void inputDialogCompleted() { mInputDialogWait.wakeOne(); }

	bool waitUntilOpen();
	inline int getConnectionId() { return mConnectionId; }

signals:
	void statusChanged();

protected:
	void startConnectionThread();		//	Always called once during the sub-class constructor
	virtual bool threadConnect() = 0;
	virtual RemoteChannel* threadOpenPrimaryChannel() = 0;
	SlaveChannel* threadOpenSlaveChannel();

	void setStatus(Status newStatus);

protected:
	RemoteConnectionThread* mThread;
	QList<RemoteChannel*> mOpenChannels;
	QString mHomeDirectory;

	bool mDeliberatelyDisconnecting;
	int mConnectionId;

	QWaitCondition mStatusWaiter;
	Status mStatus;
	QString mErrorMessage;

	QVariant mDialogParameter;
	DialogFunction mInputDialog;
	DialogCallback mDialogCallback;
	QWaitCondition mInputDialogWait;
};

#endif // REMOTECONNECTION_H
