#ifndef FTPREQUEST_H
#define FTPREQUEST_H

#include "remoterequest.h"
#include "file/location.h"
#include "file/basefile.h"
#include "rawsshconnection.h"

class FTPRequest : public RemoteRequest, public ISshConnectionCallback
{
public:
	enum Type { Ls, ReadFile, WriteFile };
	enum LsFlags { IncludeHidden = 1 };

	FTPRequest(Type type, const Location& location, BaseFile* file = 0, int flags = 0) : mType(type), mLocation(location), mFile(file), mFlags(flags) {}

	void setData(const QByteArray& data) { mData = data; }
	void setRevision(int revision) { mRevision = revision; }
	void setUndoLength(int undoLength) { mUndoLength = undoLength; }

	inline Location& getLocation() { return mLocation; }
	inline Type getType() { return mType; }
	inline int getFlags() { return mFlags; }
	inline bool getFlag(int flag) { return mFlags & flag; }
	inline BaseFile* getFile() { return mFile; }
	inline QByteArray& getData() { return mData; }
	inline int getRevision() { return mRevision; }
	inline int getUndoLength() { return mUndoLength; }

	void fileOpenProgress(int percent) { if (mFile) mFile->fileOpenProgressed(percent); }

public:
	Type mType;
	Location mLocation;
	BaseFile* mFile;
	int mFlags;
	int mRevision;
	int mUndoLength;
	QByteArray mData;
};

#endif // FTPREQUEST_H
