#include "ftpfile.h"
#include "ssh/sshhost.h"
#include "ssh/remoteconnection.h"
#include "ssh/ftpchannel.h"
#include "ssh/ftprequest.h"

FtpFile::FtpFile(const Location& location) : BaseFile(location)
{
	mHost = location.getRemoteHost();
}

void FtpFile::getChannel()
{
	mConnection = mHost->getConnection();
	if (!mConnection)
		throw("Failed to open file: failed to connect to remote host");

	mChannel = mConnection->getFtpChannel();
	if (!mChannel)
		throw("Failed to open file: failed to open an ftp channel on remote host");

	connect(mConnection, SIGNAL(statusChanged()), this, SLOT(connectionStateChanged()), Qt::QueuedConnection);
}

BaseFile* FtpFile::newFile(const QString& content)
{
	mContent = content;
	save();
	return this;
}

void FtpFile::open()
{
	setOpenStatus(BaseFile::Loading);
	getChannel();
	connectionStateChanged();

	mChannel->sendRequest(new FTPRequest(FTPRequest::ReadFile, mLocation, this));
}

void FtpFile::save()
{
	FTPRequest* request = new FTPRequest(FTPRequest::WriteFile, mLocation, this);
	request->setData(mContent.toUtf8());
	request->setUndoLength(mDocument->availableUndoSteps());
	request->setRevision(mRevision);
	mChannel->sendRequest(request);
}

bool FtpFile::canClose()
{
	return mChannel->hasPendingRequestsFor(this);
}

void FtpFile::close()
{
	setOpenStatus(Closing);
	BaseFile::closeCompleted();
}


