#ifndef SSHREQUEST_H
#define SSHREQUEST_H

#include <QByteArray>
#include <QRunnable>

#include "location.h"

enum DataType
{
	dtInt16 =    0x01,
	dtInt32 =    0x02,
	dtString =   0x03,
	dtUnsigned = 0x80
};

class SshFile;
class SshConnection;
class SshRemoteController;
class BaseFile;

//////////////////
//  Base class  //
//////////////////

class SshRequest
{
public:
	SshRequest(quint16 messageId, quint32 bufferId);
	virtual ~SshRequest();

	inline void setConnection(SshConnection* connection) { mConnection = connection; }
	inline void setController(SshRemoteController* controller) { mController = controller; }

	short getMessageId() const { return mMessageId; }
	void packMessage(QByteArray* target);
	virtual void packBody(QByteArray* target) {}
	virtual void handleResponse(const QByteArray& response) {}

	virtual void error(const QString& error) {}
	virtual void success() {}

	//	Manual work; if this ssh request does stuff like scp data down, this is where to do it.
	virtual bool hasManualComponent() { return false; }
	virtual void doManualWork(SshConnection* connection) {}

protected:
	inline void addData(QByteArray* target, unsigned char field, qint16 d) { append(target, field, dtInt16, (const char*)&d, sizeof(d)); }
	inline void addData(QByteArray* target, unsigned char field, quint16 d) { append(target, field, dtUnsigned | dtInt16, (const char*)&d, sizeof(d)); }
	inline void addData(QByteArray* target, unsigned char field, qint32 d) { append(target, field, dtInt32, (const char*)&d, sizeof(d)); }
	inline void addData(QByteArray* target, unsigned char field, quint32 d) { append(target, field, dtUnsigned | dtInt32, (const char*)&d, sizeof(d)); }
	void addData(QByteArray* target, unsigned char field, const QByteArray& d);
	void addData(QByteArray* target, unsigned char field, const char* d);
	void append(QByteArray* target, unsigned char field, unsigned char type, const char* data, unsigned int length);

	quint16 mMessageId;
	quint32 mBufferId;
	SshConnection* mConnection;
	SshRemoteController* mController;
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

	virtual void error(const QString& error);
	virtual void success();

private:
	Location mLocation;
	QList<Location> mDirList;
};

///////////////////////
//  Message 2: open  //
///////////////////////

class SshRequest_open : public SshRequest
{
public:
	SshRequest_open(SshFile* file);
	virtual void packBody(QByteArray* target);
	virtual void handleResponse(const QByteArray& response);

	virtual bool hasManualComponent() { return true; }
	virtual void doManualWork(SshConnection* connection);

	virtual void error(const QString& error);
	virtual void success();

private:
	SshFile* mFile;
	QByteArray mData;
};

////////////////////////////////
//  Message 3: change buffer  //
////////////////////////////////

class SshRequest_changeBuffer : public SshRequest
{
public:
	SshRequest_changeBuffer(quint32 bufferId, quint32 position, quint32 removeCount, const QByteArray& add);
	virtual void packBody(QByteArray* target);
	virtual void handleResponse(const QByteArray& response);

	virtual void error(const QString& error);

private:
	quint32 mPosition;
	quint32 mRemoveCount;
	QByteArray mAdd;
};

//////////////////////////////
//  Message 4: save buffer  //
//////////////////////////////

class SshRequest_saveBuffer : public SshRequest
{
public:
	SshRequest_saveBuffer(quint32 bufferId, SshFile* file, int revision, const QByteArray& fileContent);
	virtual void packBody(QByteArray* target);
	virtual void handleResponse(const QByteArray& response);

	virtual void error(const QString& error);
	virtual void success();

private:
	QByteArray mFileContent;
	SshFile* mFile;
	int mRevision;
};

#endif // SSHREQUEST_H
