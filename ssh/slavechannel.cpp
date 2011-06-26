#include "slavechannel.h"
#include "slaverequest.h"
#include "sshconnection.h"
#include <QDebug>

#define MAX_LINE_SIZE	4000

SlaveChannel::SlaveChannel(RemoteConnection* connection, bool sudo)
	: RemoteChannel(connection, (sudo ? SudoSlave : Slave))
{
	mSudo = sudo;
	startThread();
}

void SlaveChannel::handleNotification(const QByteArray& notification)
{
	qDebug() << "Received notification!";
}

void SlaveChannel::threadCheckForNotifications()
{
	//	If there is unsolicited data ready, read the full line
	if (mConnection->pollData(mRawHandle))
	{
		qDebug() << "Poll data available from the server!!";
	}
}

void SlaveChannel::threadSendMessages(QList<RemoteRequest*>& messages)
{
	//	Encode the queued up messages into a blob for sending
	QByteArray sendBlob;
	foreach (RemoteRequest* rq, messages)
		static_cast<SlaveRequest*>(rq)->packMessage(&sendBlob);
	sendBlob = sendBlob.toBase64();

	//	Break the blob up into sizes at Perl won't have a hernia over
	for (int cursor = MAX_LINE_SIZE; cursor < sendBlob.length(); cursor += MAX_LINE_SIZE)
		sendBlob.insert(cursor, '\n');
	sendBlob.append("%\n");

	mConnection->sendLine(mRawHandle, sendBlob);

	//	Wait for a response for each message in turn
	while (messages.length())
	{
		QByteArray response = QByteArray::fromBase64(mConnection->readLine(mRawHandle));

		//	First, make sure this is a response, and not a notification
		if ((unsigned char)response[0] == (unsigned char)255)
		{
			handleNotification(response);
			continue;
		}

		SlaveRequest* rq = static_cast<SlaveRequest*>(messages.takeFirst());

		try
		{
			rq->checkForErrors(response);
			rq->handleResponse(response);
			if (rq->hasManualComponent())
				rq->doManualWork(static_cast<SshConnection*>(mConnection)->getRawConnection());
			rq->handleSuccess();
		}
		catch (SlaveRequest::Error error)
		{
			rq->handleError(error);
		}
		catch (QString message)
		{
			rq->handleError(message);
		}
	}
}

void SlaveChannel::threadConnect()
{
	qDebug() << "Connecting slave channel thread...";
	mRawHandle = mConnection->createRawSlaveChannel(mSudo);
}
















