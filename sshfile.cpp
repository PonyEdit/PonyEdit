#include <QDebug>
#include "sshhost.h"
#include "sshfile.h"
#include "sshrequest.h"
#include "sshremotecontroller.h"
#include "tools.h"

SshFile::SshFile(const Location& location) : BaseFile(location)
{
	mHost = location.getRemoteHost();
	mHost->registerOpenFile(this);
	mBufferId = -1;
	mChangePumpCursor = 0;
	mChangeBufferSize = 0;

	connect(this, SIGNAL(fileOpenedRethreadSignal(QByteArray)), this, SLOT(resyncSuccess(int)), Qt::QueuedConnection);
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
		if (mLastSaveChecksum == checksum)
		{
			//	If the file's checksum matches the last save, just pump the change queue from the last save...
			movePumpCursor(mLastSavedRevision);
			setOpenStatus(Ready);
			pumpChangeQueue();
		}
		else
		{
			//	If the file's checksum does NOT match the last save, reload the entire thing in its current form...
			resync();
		}
	}
}

void SshFile::resync()
{
	setOpenStatus(Repairing);
	mController->sendRequest(new SshRequest_resyncFile(mBufferId, this, mContent, mRevision));
}

void SshFile::resyncError(const QString& error)
{
	mError = "Failed to resync with remote host!";
	setOpenStatus(SyncError);
}

void SshFile::resyncSuccess(int revision)
{
	//	If this is not the main thread, move to the main thread.
	if (!Tools::isMainThread())
	{
		emit resyncSuccessRethreadSignal(revision);
		return;
	}

	//	Find the right place to put the revision push cursor
	movePumpCursor(revision);

	setOpenStatus(Ready);
	pumpChangeQueue();
}

void SshFile::movePumpCursor(int revision)
{
	mChangePumpCursor = 0;
	while (mChangePumpCursor < mChangesSinceLastSave.length() && mChangesSinceLastSave[mChangePumpCursor]->revision <= revision)
		mChangePumpCursor++;
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



