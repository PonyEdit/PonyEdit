#include <QCryptographicHash>
#include <QDebug>
#include "xferrequest.h"

XferRequest::XferRequest( bool sudo, const QByteArray &filename, const Callback &callback ) :
	mSudo( sudo ),
	mUpload( false ),
	mFilename( filename ),
	mData(),
	mRequestHeader(),
	mChecksum(),
	mEncodedData(),
	mSize( 0 ) {

	connect( this, SIGNAL( transferSuccess( QVariantMap ) ), callback.getTarget(), callback.getSuccessSlot() );
	connect( this, SIGNAL( transferFailure( QString, int ) ), callback.getTarget(), callback.getFailureSlot() );

	if ( callback.getProgressSlot() ) {
		connect( this, SIGNAL( transferProgress( int ) ), callback.getTarget(), callback.getProgressSlot() );
	}
}

const QByteArray &XferRequest::prepareHeader() {
	mRequestHeader = mFilename + ( isUploadRequest() ? "u" : "d" ) + "\n";
	if ( isUploadRequest() ) {
		QCryptographicHash hash( QCryptographicHash::Md5 );
		hash.addData( mData );
		QByteArray checksum = hash.result().toHex().toLower();

		mRequestHeader.append( QString::number( getEncodedData().size() ) );
		mRequestHeader.append( "\n" );
		mRequestHeader.append( checksum );
		mRequestHeader.append( "\n" );
	}

	return mRequestHeader;
}

void XferRequest::handleSuccess() {
	QVariantMap result;
	result.insert( "data", mData );
	result.insert( "checksum", mChecksum );
	emit transferSuccess( result );
}

void XferRequest::handleFailure( const QString &error, int errorFlags ) {
	emit transferFailure( error, errorFlags );
}

void XferRequest::handleProgress( int percent ) {
	emit transferProgress( percent );
}
