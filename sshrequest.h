#ifndef SSHREQUEST_H
#define SSHREQUEST_H

#include <QByteArray>
#include "location.h"

enum DataType
{
	dtInt16 =    0x01,
	dtInt32 =    0x02,
	dtString =   0x03,
	dtUnsigned = 0x80
};

//////////////////
//  Base class  //
//////////////////

class SshRequest
{
public:
	SshRequest(quint16 messageId, quint32 bufferId);
	virtual ~SshRequest();

	short getMessageId() const { return mMessageId; }
	void packMessage(QByteArray* target);
	virtual void packBody(QByteArray* target) {}
	virtual void handleResponse(const QByteArray& response) {}

protected:
	inline void addData(QByteArray* target, unsigned char field, qint16 d) { append(target, field, dtInt16, (const char*)&d, sizeof(d)); }
	inline void addData(QByteArray* target, unsigned char field, quint16 d) { append(target, field, dtUnsigned | dtInt16, (const char*)&d, sizeof(d)); }
	inline void addData(QByteArray* target, unsigned char field, qint32 d) { append(target, field, dtInt32, (const char*)&d, sizeof(d)); }
	inline void addData(QByteArray* target, unsigned char field, quint32 d) { append(target, field, dtUnsigned | dtInt32, (const char*)&d, sizeof(d)); }
	void addData(QByteArray* target, unsigned char field, const char* d);
	void append(QByteArray* target, unsigned char field, unsigned char type, const char* data, unsigned int length);

private:
	quint16 mMessageId;
	quint32 mBufferId;
};

/////////////////////
//  Message 1: ls  //
/////////////////////

class SshRequest_ls : public SshRequest
{
public:
	SshRequest_ls(const Location& location);
	virtual void packBody(QByteArray* target);
	virtual void handleResponse(const QByteArray& response);

private:
	Location mLocation;
};

#endif // SSHREQUEST_H
