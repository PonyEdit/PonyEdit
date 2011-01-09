#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include <QObject>
#include <QPixmap>
#include <QWidget>
#include <QDialogButtonBox>
#include <QWaitCondition>
#include <QMutex>

class RemoteConnection;
class ConnectionStatusWidget;
typedef void(*DialogFunction)(ConnectionStatusWidget*, RemoteConnection*, QWidget*);
typedef bool(*DialogCallback)(ConnectionStatusWidget*, RemoteConnection*, QDialogButtonBox::ButtonRole);

class RemoteConnection : public QObject
{
	Q_OBJECT

public:
	enum Status
	{
		Uninitialized  = 0x0001,
		Connecting     = 0x0002,
		Authenticating = 0x0004,
		Configuring    = 0x0008,
		Connected      = 0x0010,
		Disconnecting  = 0x0020,
		Disconnected   = 0x0040,
		Error          = 0x0080,
		BaseStatusMask = 0x0FFF,

		WaitingOnInput = 0x1000
	};

	RemoteConnection();

	virtual QString getName() = 0;

	inline Status getStatus() { return mStatus; }
	inline Status getBaseStatus() { return static_cast<Status>(mStatus & BaseStatusMask); }
	inline bool isConnected() { return mStatus & Connected; }

	void setStatus(Status newStatus);
	void setErrorStatus(const QString& errorMessage);

	void waitForInput(DialogFunction dialogFunction, DialogCallback callbackFunction = DefaultDialogCallback);
	inline bool isWaitingOnInput() { return mStatus & WaitingOnInput; }
	inline void populateInputDialog(ConnectionStatusWidget* statusWidget, QWidget* target) { mInputDialog(statusWidget, this, target); }
	inline bool inputDialogCallback(ConnectionStatusWidget* statusWidget, QDialogButtonBox::ButtonRole buttonRole) { return mDialogCallback(statusWidget, this, buttonRole); }
	static bool DefaultDialogCallback(ConnectionStatusWidget*, RemoteConnection*, QDialogButtonBox::ButtonRole) { return true; }
	inline void inputDialogCompleted() { mInputDialogWait.wakeOne(); }

	void disconnect(bool deliberate);
	bool isDisconnecting();
	inline bool isDeliberatelyDisconnecting() { return mDeliberatelyDisconnecting; }

	QString getStatusString();
	QPixmap getStatusIcon();

signals:
	void statusChanged();

private:
	Status mStatus;
	QString mErrorMessage;

	DialogFunction mInputDialog;
	DialogCallback mDialogCallback;
	QWaitCondition mInputDialogWait;

	bool mDeliberatelyDisconnecting;
};

#endif // REMOTECONNECTION_H
