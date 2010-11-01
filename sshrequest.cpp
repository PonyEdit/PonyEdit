#include "sshrequest.h"
#include <QDebug>

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
//  Message 1: open  //
///////////////////////

SshRequest_open::SshRequest_open(const Location& location) : SshRequest(2, 0), mLocation(location) {}

void SshRequest_open::packBody(QByteArray* target)
{
	addData(target, 'f', (const char*)mLocation.getRemotePath().toUtf8());
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
	//	Take note of the bufferId
	//

	mBufferId = *(quint32*)cursor;
}

void SshRequest_open::doManualWork(SshConnection* connection)
{
	mData = connection->readFile(mLocation.getRemotePath().toUtf8());
}

void SshRequest_open::error(const QString& error)
{
	mLocation.fileOpenError(error);
}

void SshRequest_open::success()
{
	mLocation.sshFileOpenResponse(mBufferId, mData);
}



