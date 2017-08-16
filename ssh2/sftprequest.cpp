#include <QDebug>
#include <QRegExp>
#include "sftprequest.h"

SFTPRequest::SFTPRequest( SFTPRequest::Type type, const Callback& callback ) :
	mType( type ),
	mPath( "" ),
	mIncludeHidden( false ),
	mCallback( callback ),
	mContent(),
	mRevision( 0 ),
	mUndoLength( 0 ) {}

void SFTPRequest::setPath( const QString& path ) {
	mPath = path;
	mPath.replace( QRegExp( "^~" ), "." );
}
