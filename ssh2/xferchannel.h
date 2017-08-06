#ifndef XFERCHANNEL_H
#define XFERCHANNEL_H

#include "slavechannel.h"

class XferRequest;
class XferChannel : public SlaveChannel
{
public:
	XferChannel(SshHost* host, bool sudo);

	XferChannel(XferChannel const&) = delete;
	XferChannel& operator=(XferChannel const&) = delete;
	
	virtual Type getType() { return mSudo ? SudoXfer : Xfer; }

protected:
	virtual QByteArray getSlaveRun(bool sudo);

	virtual bool mainUpdate();

	ReadReply readBinaryData(int size);

private:
	enum InternalStatus { _WaitingForRequests, _SendingRequestHeader, _WaitingForReady,
		_ReadingDownloadHeader, _DownloadingBody, _UploadingBody, _WaitingForOk };
	InternalStatus mInternalStatus;
	XferRequest* mCurrentRequest;

	QByteArray mBinaryReadBuffer;
	bool mLeftoverEscape;
};

#endif // XFERCHANNEL_H
