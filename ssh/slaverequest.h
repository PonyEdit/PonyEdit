#ifndef SLAVEREQUEST_H
#define SLAVEREQUEST_H

#include <QByteArray>
#include <QRunnable>
#include <QString>

#include "file/location.h"
#include "remoterequest.h"
#include "rawsshconnection.h"

enum DataType
{
	dtInt16 =    0x01,
	dtInt32 =    0x02,
	dtString =   0x03,
	dtUnsigned = 0x80
};

enum FileFlags
{
	fileIsDir =    0x01,
	fileCanRead =  0x02,
	fileCanWrite = 0x04
};

class SlaveFile;
class RawSshConnection;
class SshConnection;
class BaseFile;

//////////////////
//  Base class  //
//////////////////

class SlaveRequest : public RemoteRequest
{
public:
	SlaveRequest(quint16 messageId, quint32 bufferId);
	virtual ~SlaveRequest();

	inline void setController(SshConnection* controller) { mController = controller; }

	short getMessageId() const { return mMessageId; }
	void packMessage(QByteArray* target);
	virtual void packBody(QByteArray* /*target*/) {}
	virtual void handleResponse(const QByteArray& /*response*/) {}

	//	Manual work; if this ssh request does stuff like scp data down, this is where to do it.
	virtual bool hasManualComponent() { return false; }
	virtual void doManualWork(RawSshConnection* /*connection*/) {}

	void checkForErrors(const QByteArray& response);

protected:

	//	UNSUPPORTED: Signed 16 and 32-bit numbers. Perl <5.10 can't read 'em without some manual work.
	//inline void addData(QByteArray* target, unsigned char field, qint16 d) { append(target, field, dtInt16, (const char*)&d, sizeof(d)); }
	//inline void addData(QByteArray* target, unsigned char field, qint32 d) { append(target, field, dtInt32, (const char*)&d, sizeof(d)); }

	inline void addData(QByteArray* target, unsigned char field, quint16 d) { append(target, field, dtUnsigned | dtInt16, (const char*)&d, sizeof(d)); }
	inline void addData(QByteArray* target, unsigned char field, quint32 d) { append(target, field, dtUnsigned | dtInt32, (const char*)&d, sizeof(d)); }
	void addData(QByteArray* target, unsigned char field, const QByteArray& d);
	void addData(QByteArray* target, unsigned char field, const char* d);
	void append(QByteArray* target, unsigned char field, unsigned char type, const char* data, unsigned int length);

	quint16 mMessageId;
	quint32 mBufferId;
	SshConnection* mController;
};

/////////////////////
//  Message 1: ls  //
/////////////////////

class SlaveRequest_ls : public SlaveRequest
{
public:
	SlaveRequest_ls(const Location& location);
	virtual void packBody(QByteArray* target);
	virtual void handleResponse(const QByteArray& response);

	virtual void error(const Error& err);
	virtual void success();

private:
	Location mLocation;
	QList<Location> mDirList;
};

///////////////////////
//  Message 2: open  //
///////////////////////

class SlaveRequest_open : public SlaveRequest, public ISshConnectionCallback
{
public:
	enum Fetch { Content, Checksum };

	SlaveRequest_open(SlaveFile* file, Fetch fetch);
	virtual void packBody(QByteArray* target);
	virtual void handleResponse(const QByteArray& response);

	virtual bool hasManualComponent() { return true; }
	virtual void doManualWork(RawSshConnection* connection);

	virtual void error(const Error& err);
	virtual void success();

	virtual void fileOpenProgress(int percent);

private:
	SlaveFile* mFile;
	QByteArray mData;
	QString mChecksum;
	Fetch mFetch;
	bool mReadOnly;
};

////////////////////////////////
//  Message 3: change buffer  //
////////////////////////////////

class SlaveRequest_changeBuffer : public SlaveRequest
{
public:
	SlaveRequest_changeBuffer(quint32 bufferId, quint32 position, quint32 removeCount, const QString& add);
	virtual void packBody(QByteArray* target);

	virtual void error(const Error& err);

private:
	quint32 mPosition;
	quint32 mRemoveCount;
	QString mAdd;
};

//////////////////////////////
//  Message 4: save buffer  //
//////////////////////////////

class SlaveRequest_saveBuffer : public SlaveRequest
{
public:
	SlaveRequest_saveBuffer(quint32 bufferId, SlaveFile* file, int revision, int undoLength, const QString& fileContent);
	virtual void packBody(QByteArray* target);

	virtual void error(const Error& err);
	virtual void success();

private:
	QString mFileContent;
	QByteArray mChecksum;
	SlaveFile* mFile;
	int mRevision;
	int mUndoLength;
};

////////////////////////////
//  Message 5: keepalive  //
////////////////////////////

class SlaveRequest_keepalive : public SlaveRequest
{
public:
	SlaveRequest_keepalive() : SlaveRequest(5, 0) {}
};

/////////////////////////////
//  Message 6: resyncFile  //
/////////////////////////////

class SlaveRequest_resyncFile : public SlaveRequest
{
public:
	SlaveRequest_resyncFile(quint32 bufferId, SlaveFile* file, const QString& content, int revision);
	virtual void packBody(QByteArray* target);

	virtual void error(const Error& err);
	virtual void success();

private:
	QString mContent;
	SlaveFile* mFile;
	int mRevision;
};

////////////////////////////
//  Message 7: closeFile  //
////////////////////////////

class SlaveRequest_closeFile : public SlaveRequest
{
public:
	SlaveRequest_closeFile(SlaveFile* file, quint32 bufferId);

	virtual void error(const Error& err);
	virtual void success();

private:
	SlaveFile* mFile;
};

/////////////////////////////
//  Message 8: createFile  //
/////////////////////////////

class SlaveRequest_createFile : public SlaveRequest
{
public:
	SlaveRequest_createFile(SlaveFile* file, const QString& content);
	virtual void packBody(QByteArray* target);

	virtual void error(const Error& err);
	virtual void success();

private:
	SlaveFile* mFile;
	QString mContent;
};


//////////////////////////////////
//  Message 9: createDirectory  //
//////////////////////////////////

class SlaveRequest_createDirectory : public SlaveRequest
{
public:
	SlaveRequest_createDirectory(const Location& location, QString dirName);
	virtual void packBody(QByteArray* target);

	virtual void error(const Error& err);
	virtual void success();

private:
	Location mLocation;
	QString mDirName;
};

#endif // SLAVEREQUEST_H
