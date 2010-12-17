#include <QDebug>
#include <QCryptographicHash>
#include "ssh/sshrequest.h"
#include "file/sshfile.h"
#include "ssh/sshconnection.h"
#include "main/globaldispatcher.h"

//////////////////
//  Base class  //
//////////////////

SshRequest::SshRequest(quint16 messageId, quint32 bufferId)
{
	mMessageId = messageId;
	mBufferId = bufferId;
}

SshRequest::~SshRequest() {}

void SshRequest::packMessage(QByteArray* target)
{
	target->append((const char*)&mMessageId, sizeof(mMessageId));
	target->append((const char*)&mBufferId, sizeof(mBufferId));

	int bodyLengthOffset = target->size();
	quint32 bodyLength = 0;
	target->append((const char*)&bodyLength, sizeof(quint32));

	int bodyOffset = target->size();
	packBody(target);

	quint32* bodyLengthField = (quint32*)(target->data() + bodyLengthOffset);
	*bodyLengthField = target->size() - bodyOffset;
}

void SshRequest::append(QByteArray* target, unsigned char field, unsigned char type, const char* data, unsigned int length)
{
	target->append((const char*)&field, 1);
	target->append((const char*)&type, 1);
	target->append(data, length);
}

void SshRequest::addData(QByteArray* target, unsigned char field, const QByteArray& d)
{
	quint32 len = d.length();
	unsigned char type = dtString;
	target->append((const char*)&field, 1);
	target->append((const char*)&type, 1);
	target->append((const char*)&len, 4);
	target->append(d.constData(), len);
}

void SshRequest::addData(QByteArray* target, unsigned char field, const char* d)
{
	quint32 len = strlen(d);
	unsigned char type = dtString;
	target->append((const char*)&field, 1);
	target->append((const char*)&type, 1);
	target->append((const char*)&len, 4);
	target->append(d, len);
}


/////////////////////
//  Message 1: ls  //
/////////////////////

SshRequest_ls::SshRequest_ls(const Location& location) : SshRequest(1, 0), mLocation(location) {}

void SshRequest_ls::packBody(QByteArray* target)
{
	addData(target, 'd', (const char*)mLocation.getRemotePath().toUtf8());
}

void SshRequest_ls::handleResponse(const QByteArray& response)
{
	const char* cursor = response.constData();
	const char* dataEnd = response.constData() + response.size();

	//
	//	Check for errors
	//

	bool success = *(cursor++);
	if (!success)
	{
		quint32 errorLength;
		QString errorString;

		errorLength = *(quint32*)cursor;
		cursor += 4;

		errorString = QByteArray(cursor, errorLength);
		cursor += errorLength;

		throw(errorString);
	}

	//
	//	Read directory structure
	//

	while (cursor < dataEnd)
	{
		quint32 filenameLength;
		QString filename;
		quint64 size;
		//quint64 lastModified;
		quint8 isDir;

		filenameLength = *(quint32*)cursor;
		cursor += 4;

		filename = QByteArray(cursor, filenameLength);
		cursor += filenameLength;

		isDir = *(quint8*)cursor;
		cursor += 1;

		size = *(quint32*)cursor;
		cursor += 4;

		/*lastModified = *(quint32*)cursor;
		cursor += 4;*/

		mDirList.append(Location(mLocation, mLocation.getPath() + "/" + filename, isDir?Location::Directory:Location::File, size, QDateTime()));
	}
}

void SshRequest_ls::error(const QString& error)
{
	mLocation.childLoadError(error);
}

void SshRequest_ls::success()
{
	mLocation.sshChildLoadResponse(mDirList);
}


///////////////////////
//  Message 2: open  //
///////////////////////

SshRequest_open::SshRequest_open(SshFile* file, SshRequest_open::Fetch fetch) : SshRequest(2, 0), mFile(file), mFetch(fetch) {}

void SshRequest_open::packBody(QByteArray* target)
{
	const Location& fileLocation = mFile->getLocation();
	addData(target, 'f', (const char*)fileLocation.getRemotePath().toUtf8());
}

void SshRequest_open::handleResponse(const QByteArray& response)
{
	const char* cursor = response.constData();

	//
	//	Check for errors
	//

	bool success = *(cursor++);
	if (!success)
	{
		quint32 errorLength;
		QString errorString;

		errorLength = *(quint32*)cursor;
		cursor += 4;

		errorString = QByteArray(cursor, errorLength);
		cursor += errorLength;

		throw(errorString);
	}

	//
	//	Take note of the bufferId & checksum
	//

	mBufferId = *(quint32*)cursor;
	cursor += 4;

	int checksumLength = *(quint32*)cursor;
	cursor += 4;

	mChecksum = QByteArray(cursor, checksumLength);
	cursor += checksumLength;

}

void SshRequest_open::doManualWork(SshConnection* connection)
{
	if (mFetch == Content)
	{
		const Location& fileLocation = mFile->getLocation();
		mData = connection->readFile(fileLocation.getRemotePath().toUtf8(), this);
	}
}

void SshRequest_open::error(const QString& error)
{
	mFile->openError(error);
}

void SshRequest_open::success()
{
	mFile->fileOpened(mBufferId, mData, mChecksum);
}

void SshRequest_open::fileOpenProgress(int percent)
{
	mFile->fileOpenProgressed(percent);
}

////////////////////////////////
//  Message 3: change buffer  //
////////////////////////////////

SshRequest_changeBuffer::SshRequest_changeBuffer(quint32 bufferId, quint32 position, quint32 removeCount, const QByteArray& add)
	: SshRequest(3, bufferId)
{
	mPosition = position;
	mRemoveCount = removeCount;
	mAdd = add;
}

void SshRequest_changeBuffer::packBody(QByteArray* target)
{
	addData(target, 'p', mPosition);
	addData(target, 'r', mRemoveCount);
	addData(target, 'a', mAdd);
}

void SshRequest_changeBuffer::handleResponse(const QByteArray& response)
{
	const char* cursor = response.constData();

	//
	//	Check for errors
	//

	bool success = *(cursor++);
	if (!success)
	{
		quint32 errorLength;
		QString errorString;

		errorLength = *(quint32*)cursor;
		cursor += 4;

		errorString = QByteArray(cursor, errorLength);
		cursor += errorLength;

		throw(errorString);
	}
}


void SshRequest_changeBuffer::error(const QString& error)
{
	qDebug() << "Error changing remote buffer: " << error;
}

//////////////////////////////
//  Message 4: save buffer  //
//////////////////////////////

SshRequest_saveBuffer::SshRequest_saveBuffer(quint32 bufferId, SshFile* file, int revision, const QByteArray& fileContent)
	: SshRequest(4, bufferId)
{
	mFile = file;
	mRevision = revision;
	mFileContent = fileContent;
}

void SshRequest_saveBuffer::packBody(QByteArray* target)
{
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(mFileContent);
	mChecksum = hash.result().toHex().toLower();

	addData(target, 'c', mChecksum);
}

void SshRequest_saveBuffer::handleResponse(const QByteArray& response)
{
	const char* cursor = response.constData();

	//
	//	Check for errors
	//

	bool success = *(cursor++);
	if (!success)
	{
		quint32 errorLength;
		QString errorString;

		errorLength = *(quint32*)cursor;
		cursor += 4;

		errorString = QByteArray(cursor, errorLength);
		cursor += errorLength;

		throw(errorString);
	}
}

void SshRequest_saveBuffer::error(const QString& error)
{
	gDispatcher->emitGeneralErrorMessage(QString("Failed to save ") + mFile->getLocation().getLabel() + ": " + error);
}

void SshRequest_saveBuffer::success()
{
	mFile->savedRevision(mRevision, mChecksum);
}

/////////////////////////////
//  Message 6: resyncFile  //
/////////////////////////////

SshRequest_resyncFile::SshRequest_resyncFile(quint32 bufferId, SshFile* file, const QByteArray& content, int revision)
	: SshRequest(6, bufferId)
{
	mRevision = revision;
	mFile = file;
	mContent = content;
}

void SshRequest_resyncFile::packBody(QByteArray* target)
{
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(mContent);
	QByteArray md5checksum = hash.result().toHex().toLower();

	addData(target, 'd', mContent);
	addData(target, 'c', md5checksum);
	addData(target, 's', 0);
}

void SshRequest_resyncFile::handleResponse(const QByteArray& response)
{
	const char* cursor = response.constData();

	//
	//	Check for errors
	//

	bool success = *(cursor++);
	if (!success)
	{
		quint32 errorLength;
		QString errorString;

		errorLength = *(quint32*)cursor;
		cursor += 4;

		errorString = QByteArray(cursor, errorLength);
		cursor += errorLength;

		throw(errorString);
	}
}

void SshRequest_resyncFile::error(const QString& error)
{
	mFile->resyncError(error);
}

void SshRequest_resyncFile::success()
{
	mFile->resyncSuccess(mRevision);
}

////////////////////////////
//  Message 7: closeFile  //
////////////////////////////

SshRequest_closeFile::SshRequest_closeFile(SshFile* file, quint32 bufferId)
	: SshRequest(7, bufferId)
{
	mFile = file;
}

void SshRequest_closeFile::error(const QString& /* error */)
{
	//	Don't care if this fails; the only way it can fail is if the connection drops.
	//	If the connection drops, the file is essentially closed server-side anyway.
	mFile->closeCompleted();
}

void SshRequest_closeFile::success()
{
	mFile->closeCompleted();
}









