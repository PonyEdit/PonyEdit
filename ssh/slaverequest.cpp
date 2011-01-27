#include <QDebug>
#include <QCryptographicHash>
#include "ssh/SlaveRequest.h"
#include "file/slavefile.h"
#include "main/globaldispatcher.h"

//////////////////
//  Base class  //
//////////////////

SlaveRequest::SlaveRequest(quint16 messageId, quint32 bufferId)
{
	mMessageId = messageId;
	mBufferId = bufferId;
}

SlaveRequest::~SlaveRequest() {}

void SlaveRequest::packMessage(QByteArray* target)
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

void SlaveRequest::append(QByteArray* target, unsigned char field, unsigned char type, const char* data, unsigned int length)
{
	target->append((const char*)&field, 1);
	target->append((const char*)&type, 1);
	target->append(data, length);
}

void SlaveRequest::addData(QByteArray* target, unsigned char field, const QByteArray& d)
{
	quint32 len = d.length();
	unsigned char type = dtString;
	target->append((const char*)&field, 1);
	target->append((const char*)&type, 1);
	target->append((const char*)&len, 4);
	target->append(d.constData(), len);
}

void SlaveRequest::addData(QByteArray* target, unsigned char field, const char* d)
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

SlaveRequest_ls::SlaveRequest_ls(const Location& location) : SlaveRequest(1, 0), mLocation(location) {}

void SlaveRequest_ls::packBody(QByteArray* target)
{
	addData(target, 'd', (const char*)mLocation.getRemotePath().toUtf8());
}

void SlaveRequest_ls::handleResponse(const QByteArray& response)
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
		quint8 flags;

		filenameLength = *(quint32*)cursor;
		cursor += 4;

		filename = QByteArray(cursor, filenameLength);
		cursor += filenameLength;

		flags = *(quint8*)cursor;
		cursor += 1;

		bool isDir = (flags & fileIsDir)!=0;
		bool canRead = (flags & fileCanRead)!=0;
		bool canWrite = (flags & fileCanWrite)!=0;

		size = *(quint32*)cursor;
		cursor += 4;

		/*lastModified = *(quint32*)cursor;
		cursor += 4;*/

		mDirList.append(Location(mLocation, mLocation.getPath() + "/" + filename,
			isDir?Location::Directory:Location::File, size, QDateTime(), canRead, canWrite));
	}
}

void SlaveRequest_ls::error(const QString& error)
{
	mLocation.childLoadError(error);
}

void SlaveRequest_ls::success()
{
	mLocation.sshChildLoadResponse(mDirList);
}


///////////////////////
//  Message 2: open  //
///////////////////////

SlaveRequest_open::SlaveRequest_open(SlaveFile* file, SlaveRequest_open::Fetch fetch) : SlaveRequest(2, 0), mFile(file), mFetch(fetch) {}

void SlaveRequest_open::packBody(QByteArray* target)
{
	const Location& fileLocation = mFile->getLocation();
	addData(target, 'f', (const char*)fileLocation.getRemotePath().toUtf8());
}

void SlaveRequest_open::handleResponse(const QByteArray& response)
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

	quint8 flags = *(quint8*)cursor;
	cursor++;
	mReadOnly = !(flags & fileCanWrite);

	mBufferId = *(quint32*)cursor;
	cursor += 4;

	int checksumLength = *(quint32*)cursor;
	cursor += 4;

	mChecksum = QByteArray(cursor, checksumLength);
	cursor += checksumLength;

}

void SlaveRequest_open::doManualWork(RawSshConnection* connection)
{
	if (mFetch == Content)
	{
		const Location& fileLocation = mFile->getLocation();
		mData = connection->readFile(fileLocation.getRemotePath().toUtf8(), this);
	}
}

void SlaveRequest_open::error(const QString& error)
{
	mFile->openError(error);
}

void SlaveRequest_open::success()
{
	mFile->fileOpened(mBufferId, QString::fromUtf8(mData), mChecksum, mReadOnly);
}

void SlaveRequest_open::fileOpenProgress(int percent)
{
	mFile->fileOpenProgressed(percent);
}

////////////////////////////////
//  Message 3: change buffer  //
////////////////////////////////

SlaveRequest_changeBuffer::SlaveRequest_changeBuffer(quint32 bufferId, quint32 position, quint32 removeCount, const QString& add)
	: SlaveRequest(3, bufferId)
{
	mPosition = position;
	mRemoveCount = removeCount;
	mAdd = add;
}

void SlaveRequest_changeBuffer::packBody(QByteArray* target)
{
	addData(target, 'p', mPosition);
	addData(target, 'r', mRemoveCount);
	addData(target, 'a', mAdd.toUtf8());
}

void SlaveRequest_changeBuffer::handleResponse(const QByteArray& response)
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


void SlaveRequest_changeBuffer::error(const QString& error)
{
	qDebug() << "Error changing remote buffer: " << error;
}

//////////////////////////////
//  Message 4: save buffer  //
//////////////////////////////

SlaveRequest_saveBuffer::SlaveRequest_saveBuffer(quint32 bufferId, SlaveFile* file, int revision, const QString& fileContent)
	: SlaveRequest(4, bufferId)
{
	mFile = file;
	mRevision = revision;
	mFileContent = fileContent;
}

void SlaveRequest_saveBuffer::packBody(QByteArray* target)
{
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(mFileContent.toUtf8());
	mChecksum = hash.result().toHex().toLower();

	addData(target, 'c', mChecksum);
}

void SlaveRequest_saveBuffer::handleResponse(const QByteArray& response)
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

void SlaveRequest_saveBuffer::error(const QString& /* error */)
{
	mFile->saveFailed();
}

void SlaveRequest_saveBuffer::success()
{
	mFile->savedRevision(mRevision, mChecksum);
}

/////////////////////////////
//  Message 6: resyncFile  //
/////////////////////////////

SlaveRequest_resyncFile::SlaveRequest_resyncFile(quint32 bufferId, SlaveFile* file, const QString& content, int revision)
	: SlaveRequest(6, bufferId)
{
	mRevision = revision;
	mFile = file;
	mContent = content;
}

void SlaveRequest_resyncFile::packBody(QByteArray* target)
{
	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(mContent.toUtf8());
	QByteArray md5checksum = hash.result().toHex().toLower();

	addData(target, 'd', mContent.toUtf8());
	addData(target, 'c', md5checksum);
	addData(target, 's', quint32(0));
}

void SlaveRequest_resyncFile::handleResponse(const QByteArray& response)
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

void SlaveRequest_resyncFile::error(const QString& error)
{
	mFile->resyncError(error);
}

void SlaveRequest_resyncFile::success()
{
	mFile->resyncSuccess(mRevision);
}

////////////////////////////
//  Message 7: closeFile  //
////////////////////////////

SlaveRequest_closeFile::SlaveRequest_closeFile(SlaveFile* file, quint32 bufferId)
	: SlaveRequest(7, bufferId)
{
	mFile = file;
}

void SlaveRequest_closeFile::error(const QString& /* error */)
{
	//	Don't care if this fails; the only way it can fail is if the connection drops.
	//	If the connection drops, the file is essentially closed server-side anyway.
	mFile->closeCompleted();
}

void SlaveRequest_closeFile::success()
{
	mFile->closeCompleted();
}


/////////////////////////////
//  Message 8: createFile  //
/////////////////////////////

SlaveRequest_createFile::SlaveRequest_createFile(SlaveFile* file, const QString& content) : SlaveRequest(8, 0)
{
	mFile = file;
	mContent = content;
}

void SlaveRequest_createFile::error(const QString& error)
{
	mFile->openError(error);
}

void SlaveRequest_createFile::success()
{
	mFile->open();
	Location dir = mFile->getDirectory();
	dir.asyncGetChildren(true);
}

void SlaveRequest_createFile::packBody(QByteArray* target)
{
	const Location& fileLocation = mFile->getLocation();
	addData(target, 'f', (const char*)fileLocation.getRemotePath().toUtf8());
	addData(target, 'c', mContent.toUtf8());
}

//////////////////////////////////
//  Message 9: createDirectory  //
//////////////////////////////////

SlaveRequest_createDirectory::SlaveRequest_createDirectory(const Location& location, QString dirName) : SlaveRequest(9, 0)
{
	mLocation = location;
	mDirName = dirName;
}

void SlaveRequest_createDirectory::error(const QString& error)
{
	mLocation.childLoadError(error);
}

void SlaveRequest_createDirectory::success()
{
	mLocation.asyncGetChildren(true);
}

void SlaveRequest_createDirectory::packBody(QByteArray* target)
{
	addData(target, 'l', (const char*)mLocation.getRemotePath().toUtf8());
	addData(target, 'n', (const char*)mDirName.toUtf8());
}
