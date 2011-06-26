#include <QDebug>
#include "ssh/sshhost.h"
#include "file/slavefile.h"
#include "ssh/slaverequest.h"
#include "ssh/sshconnection.h"
#include "main/tools.h"
#include "ssh/slavechannel.h"
#include "file/openfilemanager.h"
#include "filestatuswidget.h"
#include "main/dialogwrapper.h"
#include <QMessageBox>

SlaveFile::SlaveFile(const Location& location) : BaseFile(location)
{
	mHost = location.getRemoteHost();
	mHost->registerOpenFile(this);
	mBufferId = -1;
	mChangePumpCursor = 0;
	mChangeBufferSize = 0;
	mChannel = NULL;
	mConnection = NULL;

	mCreatingNewFile = false;

	connect(this, SIGNAL(resyncSuccessRethreadSignal(int)), this, SLOT(resyncSuccess(int)), Qt::QueuedConnection);
}

SlaveFile::~SlaveFile()
{
	mHost->unregisterOpenFile(this);

	foreach (Change* change, mChangesSinceLastSave) delete change;
	mChangesSinceLastSave.clear();
}

void SlaveFile::getChannel()
{
	mConnection = mHost->getConnection();
	if (!mConnection)
		throw("Failed to open file: failed to connect to remote host");

	mChannel = mLocation.isSudo() ? mConnection->getSudoChannel() : mConnection->getSlaveChannel();
	if (!mChannel)
		throw("Failed to open file: failed to open a slave channel on remote host");

	connect(mConnection, SIGNAL(statusChanged()), this, SLOT(connectionStateChanged()), Qt::QueuedConnection);
}

BaseFile* SlaveFile::newFile(const QString& content)
{
	mCreatingNewFile = true;

	getChannel();
	setOpenStatus(BaseFile::Loading);
	connectionStateChanged();

	mChannel->sendRequest(new SlaveRequest_createFile(this, content));

	return this;
}

void SlaveFile::open()
{
	if(!mCreatingNewFile)
	{
		setOpenStatus(BaseFile::Loading);
		getChannel();
		connectionStateChanged();
	}

	mChannel->sendRequest(new SlaveRequest_open(this, SlaveRequest_open::Content));
}

void SlaveFile::reconnect()
{
	//	Open the buffer remotely, ask for a checksum to be returned; not the whole file.
	setOpenStatus(Reconnecting);
	mChannel->sendRequest(new SlaveRequest_open(this, SlaveRequest_open::Checksum));
}

void SlaveFile::fileOpened(int bufferId, const QString& content, const QString& checksum, bool readOnly)
{
	mBufferId = bufferId;

	//	If this is from a plain "open" request, content will not be null.
	if (!content.isNull())
	{
		BaseFile::fileOpened(content, readOnly);
		mChangePumpCursor = 0;
	}
	else
	{
		//	This must be a reconnection attempt. Check the checksum.
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

void SlaveFile::resync()
{
	setOpenStatus(Repairing);
	mChannel->sendRequest(new SlaveRequest_resyncFile(mBufferId, this, mContent, mRevision));
}

void SlaveFile::resyncError(const QString& /*error*/)
{
	mError = "Failed to resync with remote host!";
	setOpenStatus(SyncError);
}

void SlaveFile::resyncSuccess(int revision)
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

void SlaveFile::movePumpCursor(int revision)
{
	mChangePumpCursor = 0;
	while (mChangePumpCursor < mChangesSinceLastSave.length() && mChangesSinceLastSave[mChangePumpCursor]->revision <= revision)
		mChangePumpCursor++;
}

void SlaveFile::handleDocumentChange(int position, int removeChars, const QString& insert)
{
	if (mIgnoreChanges)
		return;

	BaseFile::handleDocumentChange(position, removeChars, insert);

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

void SlaveFile::pumpChangeQueue()
{
	while (mChangePumpCursor < mChangesSinceLastSave.length())
	{
		Change* change = mChangesSinceLastSave[mChangePumpCursor];
		mChannel->sendRequest(new SlaveRequest_changeBuffer(mBufferId, change->position, change->remove, change->insert));
		mChangePumpCursor++;
	}
}

void SlaveFile::save()
{
	if(mBufferId < 0)
		return;

	mChannel->sendRequest(new SlaveRequest_saveBuffer(mBufferId, this, mRevision, mDocument->availableUndoSteps(), mContent));
}

void SlaveFile::connectionStateChanged()
{
	SshConnection::Status connectionState = mConnection->getBaseStatus();

	bool isConnected = (connectionState == SshConnection::Connected || connectionState == SshConnection::OpeningChannels);
	bool wasConnected = (mOpenStatus != Disconnected);

	if (wasConnected && !isConnected)
	{
		qDebug() << "SlaveFile disconnected";
		setOpenStatus(Disconnected);
	}
	else if (isConnected && !wasConnected)
	{
		qDebug() << "SlaveFile reconnecting... ";
		reconnect();
	}
}

void SlaveFile::setLastSavedRevision(int lastSavedRevision)
{
	BaseFile::setLastSavedRevision(lastSavedRevision);

	//	Purge all stored changes up to that point...
	while (mChangesSinceLastSave.length() > 0 && mChangesSinceLastSave[0]->revision <= mLastSavedRevision)
	{
		Change* change = mChangesSinceLastSave.takeFirst();
		mChangeBufferSize -= change->insert.size();
		mChangePumpCursor--;
		delete change;
	}

	if (mChangePumpCursor < 0) mChangePumpCursor = 0;
}

void SlaveFile::close()
{
	setOpenStatus(Closing);
	try
	{
		mChannel->sendRequest(new SlaveRequest_closeFile(this, mBufferId, true));
	}
	catch(QString &e)
	{
		qDebug() << e;
		BaseFile::closeCompleted();
	}
}

void SlaveFile::refresh()
{
	fileOpenProgressed(0);

	setOpenStatus(BaseFile::Loading);
	getChannel();
	connectionStateChanged();

	try
	{
		mChannel->sendRequest(new SlaveRequest_closeFile(this, mBufferId, false));
	}
	catch(QString &e)
	{
		qDebug() << e;
	}

	mChannel->sendRequest(new SlaveRequest_open(this, SlaveRequest_open::Content));
}

void SlaveFile::sudo()
{
	//	Close the old buffer. Don't bother checking if it succeeds; the only possible
	//	cause of failure from closing a buffer is disconnection, and that drops it anyway.
	mChannel->sendRequest(new SlaveRequest_closeFile(this, mBufferId, false));
	mHost->unregisterOpenFile(this);

	//	Break off connection with my current server
	disconnect(mConnection, SIGNAL(statusChanged()), this, SLOT(connectionStateChanged()));
	mBufferId = -1;
	mChannel = NULL;
	mConnection = NULL;

	//	Change location
	mLocation = mLocation.getSudoLocation();
	mHost = mLocation.getRemoteHost();

	//	Open a channel and begin connecting to the remote file
	getChannel();
	reconnect();

	//	Throw up a dialog to wait for the reconnection to be successful or fail
	DialogWrapper<FileStatusWidget> dialogWrapper(tr("Reopening with sudo"), new FileStatusWidget(this, NULL), false);
	if (!dialogWrapper.exec())
		throw(tr("Failed to reopen remote file."));
}











