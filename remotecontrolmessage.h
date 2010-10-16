#ifndef REMOTECONTROLMESSAGE_H
#define REMOTECONTROLMESSAGE_H

#include <QByteArray>

enum MessageType
{
	mtLs =     0x0001,
	mtOpen =   0x0002,
	mtChange = 0x0003,
	mtSave =   0x0004
};

enum DataType
{
	dtInt16 =    0x01,
	dtInt32 =    0x02,
	dtString =   0x03,
	dtUnsigned = 0x80
};

class RemoteControlMessage
{
public:
	RemoteControlMessage(MessageType type, quint32 bufferId = 0);

	inline void addData(unsigned char field, qint16 d) { append(field, dtInt16, (const char*)&d, sizeof(d)); }
	inline void addData(unsigned char field, quint16 d) { append(field, dtUnsigned | dtInt16, (const char*)&d, sizeof(d)); }
	inline void addData(unsigned char field, qint32 d) { append(field, dtInt32, (const char*)&d, sizeof(d)); }
	inline void addData(unsigned char field, quint32 d) { append(field, dtUnsigned | dtInt32, (const char*)&d, sizeof(d)); }
	void addData(unsigned char field, const char* d);

	QByteArray& finalize();

private:
	void append(unsigned char field, unsigned char type, const char* data, unsigned int length);

	MessageType mType;
	quint32 mBufferId;
	QByteArray mData;
};

#endif // REMOTECONTROLMESSAGE_H
