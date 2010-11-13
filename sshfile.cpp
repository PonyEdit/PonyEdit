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
	mChangePumpCursor = 0;
	mChangeBufferSize = 0;
}

SshFile::~SshFile()
{
	mHost->unregisterOpenFile(this);

	foreach (Change* change, mChangesSinceLastSave) delete change;
	mChangesSinceLastSave.clear();
}

void SshFile::open()
{
	if (!mHost->ensureConnection())
		throw(QString("Failed to open file: failed to connect to remote host"));
	mController = mHost->getController();

	setOpenStatus(BaseFile::Loading);

	connect(mHost->getController(), SIGNAL(stateChanged()), this, SLOT(connectionStateChanged()), Qt::QueuedConnection);
	connectionStateChanged();
	mController->sendRequest(new SshRequest_open(this, SshRequest_open::Content));
}

void SshFile::fileOpened(int bufferId, const QByteArray& content, const QString& checksum)
{
	mBufferId = bufferId;

	//	If this is from a plain "open" request, content will not be null.
	if (!content.isNull())
	{
		BaseFile::fileOpened(content);
		mChangePumpCursor = 0;
	}
	else
	{
		if (mLastSaveChecksum != checksum)
		{
			//	If the file's checksum matches the last save, just pump the change queue from the last save...
			mChangePumpCursor = mLastSavedRevision;
			setOpenStatus(Ready);
			pumpChangeQueue();
		}
		else
		{
			//	If the file's checksum does NOT match the last save, reload the entire thing in its current form...
			qDebug() << "CHECKSUM MISMATCH: NOT YET IMPLEMENTED";
		}
	}
}

void SshFile::handleDocumentChange(int position, int removeChars, const QByteArray& insert)
{
	if (mIgnoreChanges)
		return;

	BaseFile::handleDocumentChange(position, removeChars, insert);
	qDebug() << "Edit revision " << mRevision;

	mChangeBufferSize += insert.size();
	Change* change = new Change();
	change->revision = mRevision;
	change->position = position;
	change->remove = removeChars;
	change->insert = insert;
	mChangesSinceLastSave.append(change);

	if (mOpenStatus == Ready)
		pumpChangeQueue();
}

void SshFile::pumpChangeQueue()
{
	while (mChangePumpCursor < mChangesSinceLastSave.length())
	{
		Change* change = mChangesSinceLastSave[mChangePumpCursor];
		mController->sendRequest(new SshRequest_changeBuffer(mBufferId, change->position, change->remove, change->insert));
		mChangePumpCursor++;
	}
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

void SshFile::setLastSavedRevision(int lastSavedRevision)
{
	BaseFile::setLastSavedRevision(lastSavedRevision);

	qDebug() << mChangePumpCursor << mChangesSinceLastSave.length();

	//	Purge all stored changes up to that point...
	while (mChangesSinceLastSave.length() > 0 && mChangesSinceLastSave[0]->revision <= mLastSavedRevision)
	{
		Change* change = mChangesSinceLastSave.takeFirst();
		mChangeBufferSize -= change->insert.size();
		mChangePumpCursor--;
		delete change;
	}

	qDebug() << mChangePumpCursor << mChangesSinceLastSave.length();

	if (mChangeBufferSize < 0) mChangeBufferSize = 0;
	if (mChangePumpCursor < 0) mChangePumpCursor = 0;
}



