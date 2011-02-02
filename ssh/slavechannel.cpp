#include "slavechannel.h"
#include "slaverequest.h"
#include "sshconnection.h"
#include <QDebug>

SlaveChannel::SlaveChannel(RemoteConnection* connection, bool sudo)
	: RemoteChannel(connection, (sudo ? SudoSlave : Slave))
{
	mSudo = sudo;
	startThread();
}

void SlaveChannel::threadSendMessages(QList<RemoteRequest*>& messages)
{
	QByteArray sendBlob;
	foreach (RemoteRequest* rq, messages)
		static_cast<SlaveRequest*>(rq)->packMessage(&sendBlob);
	sendBlob = sendBlob.toBase64();
	sendBlob.append('\n');

	mConnection->sendLine(mRawHandle, sendBlob);

	//	Wait for a response for each message in turn
	while (messages.length())
	{
		QByteArray response = QByteArray::fromBase64(mConnection->readLine(mRawHandle));
		SlaveRequest* rq = static_cast<SlaveRequest*>(messages.takeFirst());

		try
		{
			rq->checkForErrors(response);
			rq->handleResponse(response);
			if (rq->hasManualComponent())
				rq->doManualWork(static_cast<SshConnection*>(mConnection)->getRawConnection());
			rq->success();
		}
		catch (SlaveRequest::Error error)
		{
			rq->error(error);
		}
		catch (QString message)
		{
			rq->error(message);
		}
	}
}

void SlaveChannel::threadConnect()
{
	qDebug() << "Connecting slave channel thread...";
	mRawHandle = mConnection->createRawSlaveChannel(mSudo);
}
















