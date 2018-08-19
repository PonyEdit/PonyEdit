#include <QDebug>
#include <QRegExp>
#include <utility>

#include "sftprequest.h"

SFTPRequest::SFTPRequest( SFTPRequest::Type type, Callback callback ) :
	mType( type ),
	mPath( "" ),
	mIncludeHidden( false ),
	mCallback( std::move( callback ) ),
	mContent(),
	mRevision( 0 ),
	mUndoLength( 0 ) {}

void SFTPRequest::setPath( const QString &path ) {
	mPath = path;
	mPath.replace( QRegExp( "^~" ), "." );
}
