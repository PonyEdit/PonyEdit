#ifndef XFERREQUEST_H
#define XFERREQUEST_H

#include "tools/callback.h"
#include <QByteArray>
#include <QObject>

class XferRequest : public QObject {
	Q_OBJECT

public:
	XferRequest( bool sudo, const QByteArray &filename, const Callback &callback );

	inline bool isUploadRequest() const {
		return mUpload;
	}
	inline const QByteArray &getRequestHeader() {
		return ( mRequestHeader.isNull() ? prepareHeader() : mRequestHeader );
	}

	inline void setChecksum( const QByteArray &checksum ) {
		mChecksum = checksum;
	}
	inline void setDataSize( int size ) {
		mSize = size;
	}
	inline void setData( const QByteArray &data ) {
		mData = data;
	}
	inline void setUpload( bool upload ) {
		mUpload = upload;
	}
	inline void setEncodedData( const QByteArray &encoded ) {
		mEncodedData = encoded;
	}

	inline const QByteArray &getChecksum() const {
		return mChecksum;
	}
	inline int getDataSize() const {
		return mSize;
	}
	inline const QByteArray &getData() const {
		return mData;
	}
	inline const QByteArray &getEncodedData() const {
		return mEncodedData;
	}
	inline const QByteArray &getFilename() const {
		return mFilename;
	}
	inline bool isSudo() const {
		return mSudo;
	}

	void handleSuccess();
	void handleFailure( const QString &error, int errorFlags );
	void handleProgress( int percent );

signals:
	void transferSuccess( QVariantMap result );
	void transferFailure( QString error, int errorFlags );
	void transferProgress( int percent );

protected:
	const QByteArray &prepareHeader();

private:
	bool       mSudo;
	bool       mUpload;
	QByteArray mFilename;

	QByteArray mData;
	QByteArray mRequestHeader;
	QByteArray mChecksum;
	QByteArray mEncodedData;
	int        mSize;
};

#endif // XFERREQUEST_H
