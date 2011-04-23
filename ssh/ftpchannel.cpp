#include "ftpchannel.h"
#include "ftprequest.h"
#include "sshconnection.h"
#include "file/ftpfile.h"
#include <QCryptographicHash>

FTPChannel::FTPChannel(RemoteConnection* connection) : RemoteChannel(connection, FTP)
{
	startThread();
}

void FTPChannel::threadSendMessages(QList<RemoteRequest*>& messages)
{
	foreach (RemoteRequest* rq, messages)
	{
		FTPRequest* request = static_cast<FTPRequest*>(rq);
		Location requestLoc = request->getLocation();

		switch (request->getType())
		{
			case FTPRequest::Ls:
				requestLoc.sshChildLoadResponse(mConnection->cthGetFTPListing(mRawHandle, requestLoc, request->getFlag(FTPRequest::IncludeHidden)));
				break;

			case FTPRequest::ReadFile:
			{
				QByteArray content = mConnection->cthReadFTPFile(mRawHandle, requestLoc, request);
				request->getFile()->fileOpened(content, false);
				break;
			}

			case FTPRequest::WriteFile:
				try
				{
					mConnection->cthWriteFTPFile(mRawHandle, requestLoc, request->getData());

					QCryptographicHash hash(QCryptographicHash::Md5);
					hash.addData(request->getData());
					QByteArray checksum = hash.result().toHex().toLower();

					request->getFile()->savedRevision(request->getRevision(), request->getUndoLength(), checksum);
				}
				catch (QString error)
				{
					request->getFile()->saveFailed(error, false);
				}
				break;
		}
	}
}

void FTPChannel::threadConnect()
{
	mRawHandle = mConnection->createRawFTPChannel();
}

bool FTPChannel::hasPendingRequestsFor(FtpFile* file)
{
	bool found = false;
	mRequestQueueLock.lock();
	foreach (RemoteRequest* rq, mRequestQueue)
	{
		FTPRequest* request = static_cast<FTPRequest*>(rq);
		if (request->getFile() == file)
		{
			found = true;
			break;
		}
	}
	mRequestQueueLock.unlock();
	return found;
}
















