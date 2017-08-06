#include "sftprequest.h"
#include <QDebug>
#include <QRegExp>

SFTPRequest::SFTPRequest( SFTPRequest::Type type, const Callback &callback )
    : mType( type ), mCallback( callback ) {
}

void SFTPRequest::setPath( const QString &path ) {
	mPath = path;
	mPath.replace( QRegExp( "^~" ), "." );
}
