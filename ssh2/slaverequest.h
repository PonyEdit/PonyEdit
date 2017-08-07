#ifndef SLAVEREQUEST_H
#define SLAVEREQUEST_H

#include <QByteArray>
#include <QObject>
#include <QVariant>
#include "file/slavefile.h"
#include "tools/callback.h"

class SlaveRequest : QObject
{
Q_OBJECT

public:
enum ErrorFlags { PermissionError = 0x01, ConnectionError = 0x02 };

explicit SlaveRequest( SlaveFile* file, const QByteArray& request, const QVariant& parameters,
                       const Callback &callback );

inline void setMessageId( int messageId ) {
	mMessageId = messageId;
}

inline int getMessageId() const {
	return mMessageId;
}

inline SlaveFile* getFile() const {
	return mFile;
}

inline const QByteArray& getRequest() const {
	return mRequest;
}

// Requests to open files don't pass a file ptr in to the constructor; the attach this separate opening file pointer.
// This is because slave channels use the mFile pointer to lock file-bound requests to the channels with corresponding
// bufferIds.
inline SlaveFile* getOpeningFile() const {
	return mOpeningFile;
}

inline void setOpeningFile( SlaveFile* file ) {
	mOpeningFile = file;
}

inline const QByteArray& getPackedRequest( int bufferId ) {
	return mPackedRequest.isNull() ? prepare( bufferId ) : mPackedRequest;
}

void handleReply( const QVariantMap& reply );
void failRequest( const QString& error, int errorFlags );

signals:
void requestSuccess( QVariantMap results );
void requestFailure( QString error, int errorFlags );

private:
const QByteArray& prepare( int bufferId );

QPointer< SlaveFile > mFile;
QPointer< SlaveFile > mOpeningFile;

QByteArray mRequest;
QVariant mParameters;

int mMessageId;

QByteArray mPackedRequest;
};

#endif	// SLAVEREQUEST_H
