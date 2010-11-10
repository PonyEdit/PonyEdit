#include <QDebug>
#include "sshhost.h"
#include "sshfile.h"
#include "sshrequest.h"
#include "sshremotecontroller.h"

SshFile::SshFile(const Location& location) : BaseFile(location)
{
	mHost = location.getRemoteHost();
	mHost->registerOpenFile(this);
	mBufferId = -1;
}

SshFile::~SshFile()
{
	mHost->unregisterOpenFile(this);
}

void SshFile::open()
{
	if (!mHost->ensureConnection())
		throw(QString("Failed to open file: failed to connect to remote host"));

	setOpenStatus(BaseFile::Opening);

	connect(mHost->getController(), SIGNAL(stateChanged()), this, SLOT(connectionStateChanged()), Qt::QueuedConnection);
	connectionStateChanged();
	mHost->getController()->sendRequest(new SshRequest_open(this, SshRequest_open::Content));
}

void SshFile::fileOpened(int bufferId, const QByteArray& content, const QString& checksum)
{
	mBufferId = bufferId;

	//	If this is from a plain "open" request, content will not be null.
	if (!content.isNull())
		BaseFile::fileOpened(content);
	else
	{
		//	Test that the checksum matches ok, otherwise push this content to the slave.
		if (getChecksum() != checksum.toLower())
			pushContentToSlave();

		//	Regardless of the "push content to slave" situation; mark the file as open for business. All edits should now be pushed.
		setOpenStatus(Open);
	}
}

void SshFile::handleDocumentChange(int position, int removeChars, const QByteArray& insert)
{
	BaseFile::handleDocumentChange(position, removeChars, insert);
	qDebug() << "Edit revision " << mRevision;

	if (mOpenStatus != Open)
		return;

	if (!mHost->ensureConnection())
		throw(QString("Failed to update file: failed to connect to remote host"));

	mHost->getController()->sendRequest(new SshRequest_changeBuffer(mBufferId, position, removeChars, insert));
}

void SshFile::savedRevision(int revision)
{
	mLastSavedRevision = revision;
	qDebug() << "Saved revision " << revision;
}

void SshFile::save()
{
	if (!mHost->ensureConnection())
		throw(QString("Failed to update file: failed to connect to remote host"));

	qDebug() << "Saving revision " << mRevision;
	mHost->getController()->sendRequest(new SshRequest_saveBuffer(mBufferId, this, mRevision, mContent));
}

void SshFile::pushContentToSlave()
{
	if (!mHost->ensureConnection())
		throw(QString("Failed to update file: failed to connect to remote host"));

	mHost->getController()->sendRequest(new SshRequest_pushContent(mBufferId, this, mContent));
}

void SshFile::contentPushError(const QString& error)
{
	qDebug() << "Error pushing content: " << error;
}

void SshFile::contentPushSuccess()
{
	qDebug() << "Done pushing content.";
}

void SshFile::connectionStateChanged()
{
	SshRemoteController::Status controllerState = mHost->getController()->getStatus();

	bool isConnected = (controllerState == SshRemoteController::Connected);
	bool wasConnected = (mOpenStatus != Disconnected);

	if (wasConnected && !isConnected)
		setOpenStatus(Disconnected);
	else if (isConnected && !wasConnected)
	{
		//	If reconnecting, reopen the file and just ask for a checksum to be returned; not the whole file.
		setOpenStatus(Reconnecting);
		mHost->getController()->sendRequest(new SshRequest_open(this, SshRequest_open::Checksum));
	}
}





