#ifndef SFTPREQUEST_H
#define SFTPREQUEST_H

#include <QByteArray>
#include <QVariantMap>
#include "tools/callback.h"

class SFTPRequest
{
public:
enum Type { Ls, MkDir, ReadFile, WriteFile };

SFTPRequest( Type type, const Callback& callback );

inline Type getType() const {
	return mType;
}

void setPath( const QString& path );
inline const QString& getPath() const {
	return mPath;
}

inline void setContent( const QByteArray& content ) {
	mContent = content;
}

inline void addContent( const char* data, int length ) {
	mContent.append( data, length );
}

inline const QByteArray& getContent() const {
	return mContent;
}

inline void setRevision( int revision ) {
	mRevision = revision;
}

inline int getRevision() const {
	return mRevision;
}

inline void setUndoLength( int undoLength ) {
	mUndoLength = undoLength;
}

inline int getUndoLength() const {
	return mUndoLength;
}

inline void setIncludeHidden( bool includeHidden ) {
	mIncludeHidden = includeHidden;
}

inline bool getIncludeHidden() const {
	return mIncludeHidden;
}

inline void triggerSuccess( const QVariantMap& result ) {
	mCallback.triggerSuccess( result );
}

inline void triggerFailure( const QString& error, int flags = 0 ) {
	mCallback.triggerFailure( error, flags );
}

inline void triggerProgress( int progress ) {
	mCallback.triggerProgress( progress );
}

private:
Type mType;
QString mPath;
bool mIncludeHidden;
Callback mCallback;
QByteArray mContent;
int mRevision;
int mUndoLength;
};

#endif	// SFTPREQUEST_H
