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
	QList<Location> dirList;

	const char* cursor = response.constData();
	const char* dataEnd = response.constData() + response.size();

	cursor++;
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

		dirList.append(Location(mLocation, mLocation.getPath() + "/" + filename, isDir?Location::Directory:Location::File, size, QDateTime()));
	}

	mLocation.sshChildLoadResponse(dirList);
}

