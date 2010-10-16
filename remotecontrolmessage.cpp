#include "remotecontrolmessage.h"
#include <QDebug>

RemoteControlMessage::RemoteControlMessage(MessageType type, quint32 bufferId)
{
	mType = type;
	mBufferId = bufferId;

	quint16 shortType = (quint16)type;
	quint32 lengthPlaceholder = 0;

	mData.append((const char*)&shortType, sizeof(quint16));
	mData.append((const char*)&mBufferId, sizeof(quint32));
	mData.append((const char*)&lengthPlaceholder, sizeof(quint32));
}

void RemoteControlMessage::append(unsigned char field, unsigned char type, const char* data, unsigned int length)
{
	mData.append((const char*)&field, 1);
	mData.append((const char*)&type, 1);
	mData.append(data, length);
}

QByteArray& RemoteControlMessage::finalize()
{
	//	Slot the length of the data into the length field
	quint32* lengthField = (quint32*)(mData.data() + 6);
	*lengthField = mData.length() - 10;

	return mData;
}

void RemoteControlMessage::addData(unsigned char field, const char* d)
{
	quint32 len = strlen(d);
	unsigned char type = dtString;
	mData.append((const char*)&field, 1);
	mData.append((const char*)&type, 1);
	mData.append((const char*)&len, 4);
	mData.append(d, len);
}
