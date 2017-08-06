#ifndef SLAVECHANNEL_H
#define SLAVECHANNEL_H

#include "shellchannel.h"
#include <QByteArray>
#include <QMap>
#include <QSet>

class SshHost;
class SlaveFile;
class SlaveRequest;
class SlaveChannel : public ShellChannel
{
    Q_OBJECT
public:
	explicit SlaveChannel(SshHost* host, bool sudo);
	~SlaveChannel();

	SlaveChannel(SlaveChannel const&) = delete;
	SlaveChannel& operator=(SlaveChannel const&) = delete;
			
	bool update();

	static void initialize();
	virtual Type getType() { return mSudo ? SudoSlave : Slave; }

	inline bool handlesFileBuffer(SlaveFile* file) { return mBufferIds.contains(file); }

signals:
	void channelShutdown();	//	Used to signal associated SlaveFiles.

protected:
	void shellReady();
	void finalizeSlaveInit(const QByteArray& initString);

	virtual QByteArray getSlaveRun(bool sudo);

	virtual bool mainUpdate();

	virtual int getConnectionScore();
	virtual QString getConnectionDescription();

	void criticalError(const QString& error);

protected:
	enum InternalStatus { _WaitingForShell = 40, _CheckingSlave = 41, _CheckingSlaveResponse = 42,
		_StartingSlaveUploader = 43, _WaitingForSlaveUploader = 44, _UploadingSlaveScript = 45, _WaitingForSlaveUploadResponse = 46,
		_SendingSudoPassword = 47, _WaitingForRequests = 48, _SendingRequest = 49 };

	bool handleOpening();
	void setInternalStatus(InternalStatus newStatus);

	InternalStatus mInternalStatus;
	SlaveRequest* mCurrentRequest;
	int mNextMessageId;

	bool mSudo;
	QByteArray mSudoPasswordAttempt;
	bool mTriedSudoPassword;

	QMap<int, SlaveRequest*> mRequestsAwaitingReplies;
	QMap<SlaveFile*, int> mBufferIds;

	static QByteArray sSlaveScript;

	static QByteArray sSlaveChannelInit;
	static QByteArray sSlaveUpload;
};

#endif // SLAVECHANNEL_H
