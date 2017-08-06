#include <QDebug>
#include <QCryptographicHash>
#include "file/slavefile.h"
#include "main/tools.h"
#include "file/openfilemanager.h"
#include "filestatuswidget.h"
#include "main/dialogwrapper.h"
#include <QMessageBox>
#include "ssh2/sshhost.h"
#include "ssh2/slaverequest.h"
#include "main/globaldispatcher.h"
#include "main/mainwindow.h"

SlaveFile::SlaveFile(const Location& location) : BaseFile(location),
    mHost(location.getRemoteHost()),
    mChangesSinceLastSave(),
    mChangePumpCursor(0),
    mSlaveOpenResults(),
    mDownloadedData(),
    mDownloadedChecksum()
{}

SlaveFile::~SlaveFile()
{
	foreach (Change* change, mChangesSinceLastSave) delete change;
	mChangesSinceLastSave.clear();
}

BaseFile* SlaveFile::newFile(const QString& content)
{
	SSHLOG_TRACE(mHost) << "Creating a new slave file" << content.length() << "bytes long";

	setOpenStatus(BaseFile::Loading);
	clearTempOpenData();

	//	Retrieve the file content on a separate SSH channel...
    mHost->setFileContent(mLocation.isSudo(), mLocation.getRemotePath().toLatin1(), content.toUtf8(), Callback(this, SLOT(createSuccess(QVariantMap)), SLOT(openFailure(QString,int)), SLOT(downloadProgress(int))));

	return this;
}

void SlaveFile::createSuccess(QVariantMap /* response */)
{
	SSHLOG_TRACE(mHost) << "Remote file creation successful.";

	open();
	gMainWindow->openSingleFile(mLocation);
}

void SlaveFile::open()
{
	setOpenStatus(BaseFile::Loading);
	clearTempOpenData();

	//	Tell the Slave script to open the file as a buffer
	QMap<QString, QVariant> params;
	params.insert("file", mLocation.getRemotePath());
	mHost->sendSlaveRequest(mLocation.isSudo(), this, "open", QVariant(params), Callback(this, SLOT(slaveOpenSuccess(QVariantMap)), SLOT(openFailure(QString,int))));

	//	Retrieve the file content on a separate SSH channel...
    mHost->getFileContent(mLocation.isSudo(), mLocation.getRemotePath().toLatin1(), Callback(this, SLOT(downloadSuccess(QVariantMap)), SLOT(openFailure(QString,int)), SLOT(downloadProgress(int))));
}

void SlaveFile::downloadProgress(int percent)
{
	setProgress(percent);
}

void SlaveFile::slaveOpenSuccess(QVariantMap results)
{
	if (getOpenStatus() != BaseFile::Loading)
	{
		//	Something went wrong while d/ling the file content, but the slave open worked :-/
		//	TODO: Tell the slave to close this file; it's useless to me now.
		clearTempOpenData();
		return;
	}

	mSlaveOpenResults = results;
	finalizeFileOpen();
}

void SlaveFile::downloadSuccess(QVariantMap result)
{
	if (getOpenStatus() != BaseFile::Loading)
	{
		//	Something went wrong while asking the slave to open the file, but d/l worked :-/
		clearTempOpenData();
		return;
	}

	mDownloadedData = result.value("data").toByteArray();
	mDownloadedChecksum = result.value("checksum").toByteArray();
	finalizeFileOpen();
}

void SlaveFile::finalizeFileOpen()
{
	//	Don't finalize file open until both the slave has opened it, and the file is downloaded.
	//	Check mDownloadedChecksum instead of mDownloadedData, as the latter will be null for an empty file.
	if (mSlaveOpenResults.isEmpty() || mDownloadedChecksum.isNull()) return;

	//	Ensure the checksum from the slave matches the checksum from the download
	if (mSlaveOpenResults.value("checksum").toByteArray() != mDownloadedChecksum)
	{
		openFailure("Checksum error: Slave script disagrees with the downloaded copy", 0);
		clearTempOpenData();
		return;
	}

	bool readOnly = !mSlaveOpenResults.value("writable").toBool();
	BaseFile::openSuccess(QString::fromUtf8(mDownloadedData), mDownloadedChecksum, readOnly);
	mChangePumpCursor = 0;

	clearTempOpenData();
}

void SlaveFile::slaveChannelFailure()
{
	SSHLOG_ERROR(mHost) << "File" << mLocation.getLabel() << "is initiating a reconnection after slave channel failure";
	reconnect();
}

void SlaveFile::reconnect()
{
	setOpenStatus(Reconnecting);
	clearTempOpenData();

	//	Tell the slave to open the file as a buffer
	QMap<QString, QVariant> params;
	params.insert("file", mLocation.getRemotePath());
	mHost->sendSlaveRequest(mLocation.isSudo(), this, "open", QVariant(params), Callback(this, SLOT(slaveReconnectSuccess(QVariantMap)), SLOT(slaveReconnectFailure(QString,int))));
}

void SlaveFile::slaveReconnectSuccess(QVariantMap results)
{
	//	Great! Make sure the returned checksum matches what we expect the file to contain.
	if (results.value("checksum").toByteArray() != mLastSaveChecksum)
	{
		//	Going to have to fully re-upload the file.
	}
	else
	{
		//	Just need to resume pumping updates since the last save.
		movePumpCursor(mLastSavedRevision);
		setOpenStatus(Ready);
		pumpChangeQueue();
	}
}

void SlaveFile::slaveReconnectFailure(QString error, int flags)
{
	SSHLOG_ERROR(mHost) << "Failed to reconnect to remote slave for file" << mLocation.getLabel();

	//	TODO: Special case permission errors into SUDO system.
	//	If this is a connection issue, try again. Else, report the error to the user.
	if (flags & SlaveRequest::ConnectionError)
		reconnect();
	else
		gDispatcher->emitGeneralErrorMessage(mLocation.getLabel() + " has encountered a serious error, and cannot be recovered. " +
			"Please save the file under a different name or on a different host, close it and try reconnecting. Error: " + error);
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
		Change* change = mChangesSinceLastSave[mChangePumpCursor++];

		//	Send this change to the remote server
		QMap<QString, QVariant> params;
		params.insert("p", change->position);
		if (change->remove > 0) params.insert("d", change->remove);
		if (change->insert.length() > 0) params.insert("a", change->insert);

		mHost->sendSlaveRequest(mLocation.isSudo(), this, "change", QVariant(params), Callback(this, NULL, SLOT(changePushFailure(QString,int))));
	}
}

void SlaveFile::changePushFailure(QString error, int /*flags*/)
{
	SSHLOG_ERROR(mHost) << "Slave push error for file" << mLocation.getLabel() << ": " << error;
}

void SlaveFile::save()
{
	QMap<QString, QVariant> params;
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(mContent.toUtf8());
	params.insert("checksum", hash.result().toHex().toLower());
	params.insert("revision", mRevision);
	params.insert("undoLength", mDocument->availableUndoSteps());

	mHost->sendSlaveRequest(mLocation.isSudo(), this, "save", QVariant(params), Callback(this, SLOT(slaveSaveSuccess(QVariantMap)), SLOT(slaveSaveFailure(QString,int))));
}

void SlaveFile::slaveSaveSuccess(QVariantMap results)
{
	int revision = results.value("revision").toInt();
	int undoLength = results.value("undoLength").toInt();
	QByteArray checksum = results.value("checksum").toByteArray();

	savedRevision(revision, undoLength, checksum);
}

void SlaveFile::slaveSaveFailure(QString error, int flags)
{
	saveFailure(error, flags & SlaveRequest::PermissionError);
}

void SlaveFile::setLastSavedRevision(int lastSavedRevision)
{
	BaseFile::setLastSavedRevision(lastSavedRevision);

	//	Purge all stored changes up to that point...
	while (mChangesSinceLastSave.length() > 0 && mChangesSinceLastSave[0]->revision <= mLastSavedRevision)
	{
		Change* change = mChangesSinceLastSave.takeFirst();
		mChangePumpCursor--;
		delete change;
	}

	if (mChangePumpCursor < 0) mChangePumpCursor = 0;
}

void SlaveFile::close()
{
	setOpenStatus(Closing);

	//	Just send and assume success.
	mHost->sendSlaveRequest(mLocation.isSudo(), this, "close");
	closeCompleted();
}

void SlaveFile::refresh()
{
	mHost->sendSlaveRequest(mLocation.isSudo(), this, "close");
	open();
}

void SlaveFile::sudo()
{
	//	Close the old buffer. Don't bother checking if it succeeds.
	mHost->sendSlaveRequest(false, this, "close");
	gOpenFileManager.deregisterFile(this);

	//	Change location
	mLocation = mLocation.getSudoLocation();
	mHost = mLocation.getRemoteHost();

	//	Reconnect like this was a dropout
	reconnect();
}











