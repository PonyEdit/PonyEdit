#include <QDebug>
#include <QDir>
#include <QFileIconProvider>
#include <QMessageBox>
#include <QMetaMethod>
#include <QObject>
#include <QSettings>

#include "file/favoritelocationdialog.h"
#include "file/location.h"
#include "file/openfilemanager.h"
#include "file/slavefile.h"
#include "main/global.h"
#include "main/globaldispatcher.h"
#include "ssh2/sftprequest.h"
#include "ssh2/slaverequest.h"
#include "ssh2/sshhost.h"

#ifdef Q_OS_WIN32
	#include <windows.h>
#endif

QRegExp gSshServerRegExp( "^(?:([^@:]+)@)?([a-zA-Z0-9_\\-.]{2,}):(.+)?" );
QRegExp gSftpServerRegExp( "^sftp://(([^@/]+)@)?([^/]+)(.*)" );
QRegExp gLocalPathSeparators( "/" );
QRegExp gSshPathSeparators( "[/:]" );
QList< Location::Favorite > Location::sFavorites;


///////////////////////////
// Icon Provider stuff  //
///////////////////////////

QFileIconProvider* sIconProvider = NULL;

void LocationShared::initIconProvider() {
	if ( ! sIconProvider ) {
		sIconProvider = new QFileIconProvider();
	}
}

void LocationShared::cleanupIconProvider() {
	if ( sIconProvider ) {
		delete( sIconProvider );
		sIconProvider = NULL;
	}
}

//////////////////////////////////
// Constructors & Destructors  //
//////////////////////////////////

Location::Location() {
	mData = NULL;
}

Location::Location( const Location& other ) {
	mData = other.mData;
	if ( mData ) {
		mData->mReferences++;
	}
}

Location& Location::operator=( const Location& other ) {
	if ( mData != other.mData && mData != NULL ) {
		mData->mReferences--;
		if ( mData->mReferences <= 0 ) {
			delete( mData );
		}
	}

	mData = other.mData;
	mData->mReferences++;

	return *this;
}

Location::Location( LocationShared* data ) {
	mData = data;
	mData->mReferences++;
}

Location::Location( const QString& path ) {
	mData = new LocationShared();
	mData->setPath( path );
}

Location::Location( const Location& parent,
                    const QString& path,
                    Type type,
                    int size,
                    QDateTime lastModified,
                    bool canRead,
                    bool canWrite ) {
	mData = new LocationShared();
	mData->setPath( path );
	mData->mType = type;
	mData->mSize = size;
	mData->mLastModified = lastModified;
	mData->mSelfLoaded = true;
	mData->mParent = parent;
	mData->mCanRead = canRead | canWrite;
	mData->mCanWrite = canWrite;
}

LocationShared::LocationShared() {
	initIconProvider();

	mHost = NULL;
	mSlaveChannel = NULL;
	mReferences = 1;
	mType = Location::Unknown;
	mSize = -1;
	mSelfLoaded = false;
	mRemoteHost = NULL;
	mSudo = false;
}

Location::~Location() {
	if ( mData != NULL ) {
		mData->mReferences--;
		if ( mData->mReferences <= 0 ) {
			delete ( mData );
		}
	}
}

/////////////////////////////
// Method Implementation   //
/////////////////////////////

const QString& Location::getPath() const {
	return mData->mPath;
}

const QString& Location::getLabel() const {
	return mData->mLabel;
}

bool Location::isNull() const {
	return ( mData == NULL || getPath() == "" );
}

int Location::getSize() const {
	return mData->mSize;
}

const QDateTime& Location::getLastModified() const {
	return mData->mLastModified;
}

bool Location::canRead() const {
	return mData->mCanRead;
}

bool Location::canWrite() const {
	return mData->mCanWrite;
}

bool Location::isHidden() const {
#ifdef Q_OS_WIN32
	if ( mData->mProtocol == Local ) {
		WCHAR* wchar = ( WCHAR * ) malloc( ( mData->mPath.length() + 1 ) * sizeof( WCHAR ) );
		wchar[mData->mPath.toWCharArray( wchar )] = 0;
		DWORD result = GetFileAttributes( wchar );
		delete wchar;

		return result & FILE_ATTRIBUTE_HIDDEN;
	} else
#endif
	return ( mData->mLabel.startsWith( '.' ) );
}

QString Location::getHostName() const {
	switch ( mData->mProtocol ) {
	case Local:
		return QObject::tr( "Local Computer" );

	case Ssh:
		return mData->mRemoteUserName + ( mData->mSudo ? "*@" : "@" ) + mData->mRemoteHostName;

	case Sftp:
		return "sftp://" + mData->mRemoteUserName + "@" + mData->mRemoteHostName;

	case Unsaved:
		return QObject::tr( "New Files" );

	default:
		throw( QObject::tr( "Unknown protocol" ) );
	}
}

QString Location::getHostlessPath() const {
	switch ( mData->mProtocol ) {
	case Local:
		return mData->mPath;

	case Ssh:
	case Sftp:
		return mData->mRemotePath;

	default:
		throw( QObject::tr( "Unknown protocol" ) );
	}
}

bool Location::operator==( const Location& other ) const {
	return mData->mPath == other.mData->mPath;
}

const Location& Location::getDirectory() const {
	if ( isDirectory() ) {
		return *this;
	} else {
		return getParent();
	}
}

const Location& Location::getParent() const {
	if ( mData->mParent.isNull() ) {
		QString path = getParentPath();

		// If the path is empty, it's a New File, so just return a copy of this Location
		if ( path.isEmpty() ) {
			mData->mParent = Location( *this );
		} else {
			mData->mParent = Location( path );
		}
	}

	return mData->mParent;
}

QString Location::getParentPath() const {
	QString parentPath = getPath();

	// If this is a home dir on a remote path, substitute for the remote home dir path first.
	if ( mData->mHost && parentPath.endsWith( ":~" ) ) {
		parentPath.replace( QRegExp( "~$" ), mData->mHost->getHomeDirectory() );
	}

	int lastSlash = parentPath.lastIndexOf( '/' );
	if ( lastSlash < 0 ) {
		return "";
	} else {
		parentPath.truncate( lastSlash );
		return parentPath;
	}
}

QString Location::getDisplayPath() const {
	QString p = getPath();

#ifdef Q_OS_WIN
	if ( mData->mProtocol == Local ) {
		p.replace( '/', '\\' );
	}
#endif

	return p;
}

QIcon Location::getIcon() const {
	// TODO: So far only Linux can provide icons for remote files without pooping its pants.
	// do something nicer for Windows.
	switch ( mData->mProtocol ) {
	case Local:
		return sIconProvider->icon( QFileInfo( mData->mPath ) );

	case Ssh:
	case Sftp:
		if ( isDirectory() ) {
			return sIconProvider->icon( QFileIconProvider::Folder );
		} else {
			return sIconProvider->icon( QFileIconProvider::File );
		}

	default:
		return QIcon();
	}
}

Location::Type Location::getType() const {
	if ( ! mData->mSelfLoaded && mData->mProtocol == Local ) {
		mData->localLoadSelf();
	}

	return mData->mType;
}

bool Location::isSudo() const {
	return mData->mSudo;
}

bool Location::isDirectory() const {
	if ( ! mData->mSelfLoaded && mData->mProtocol == Local ) {
		mData->localLoadSelf();
	}

	return mData->mType == Directory;
}

void LocationShared::setPath( const QString &path ) {
	mPath = path.trimmed();
	mPath.replace( '\\', '/' );

	// Clean off any trailing slashes
	while ( mPath.endsWith( '/' ) && mPath.length() > 1 ) {
		mPath.truncate( mPath.length() - 1 );
	}

	// Work out what kind of path this is. Default if no pattern matches, is local.
	if ( gSftpServerRegExp.indexIn( mPath ) > -1 ) {
		mProtocol = Location::Sftp;
		QStringList parts = gSftpServerRegExp.capturedTexts();
		mRemoteUserName = parts[2];
		mRemoteHostName = parts[3];
		mRemotePath = parts[4];

		if ( mRemotePath.length() == 0 ) {
			mRemotePath = "/";
		}
		mRemotePath.replace( QRegExp( "^/~" ), "~" );
	} else if ( gSshServerRegExp.indexIn( mPath ) > -1 ) {
		mProtocol = Location::Ssh;
		QStringList parts = gSshServerRegExp.capturedTexts();
		mRemoteUserName = parts[1];
		mRemoteHostName = parts[2];
		mRemotePath = parts[3];

		if ( mRemotePath.length() == 0 ) {
			mRemotePath = "/";
		}

		if ( mRemoteUserName.endsWith( '*' ) ) {
			mSudo = true;
			mRemoteUserName.truncate( mRemoteUserName.length() - 1 );
		}
	} else if ( mPath.length() == 0 ) {
		mProtocol = Location::Unsaved;
	} else {
		mProtocol = Location::Local;
	}

	// Work out what to label this path...
	int lastSeparatorIndex = mPath.lastIndexOf(
		mProtocol == Location::Ssh || mProtocol == Location::Sftp ? gSshPathSeparators : gLocalPathSeparators );
	mLabel = mPath.mid( lastSeparatorIndex + 1 );
	if ( mPath == "/" ) {
		mLabel = "Root (/)";
	} else if ( mPath.isEmpty() ) {
		mLabel = QString( "New File %1" ).arg( gOpenFileManager.newFileNumber() );
	}
}

void LocationShared::localLoadSelf() {
	QFileInfo fileInfo = QFileInfo( mPath );
	mType = fileInfo.isDir() ? Location::Directory : Location::File;
	mSelfLoaded = true;
}

void LocationShared::localLoadListing( bool includeHidden ) {
	if ( ! mSelfLoaded ) {
		localLoadSelf();
	}

	QList< Location > children;

	QDir directory( mPath + ( mPath.endsWith( ":" ) ? "/" : "" ) );
	QDir::Filters filters = QDir::AllEntries | QDir::NoDotAndDotDot;
	if ( includeHidden ) {
		filters |= QDir::Hidden;
	}

	QStringList entries = directory.entryList( filters );
	foreach ( QString entry, entries ) {
		if ( ! includeHidden && entry.startsWith( '.' ) ) {
			continue;
		}

		QFileInfo fileInfo( mPath + "/" + entry );
		children.append( Location( Location( this ),
		                           fileInfo.absoluteFilePath(),
		                           fileInfo.isDir() ? Location::Directory : Location::File,
		                           fileInfo.size(),
		                           fileInfo.lastModified(),
		                           fileInfo.isReadable(),
		                           fileInfo.isWritable() ) );
	}

	gDispatcher->emitLocationListSuccess( children, mPath );
}

void Location::asyncGetChildren( bool includeHidden ) {
	switch ( mData->mProtocol ) {
	case Local:
		mData->localLoadListing( includeHidden );
		break;

	case Ssh:
		mData->sshLoadListing( includeHidden );
		break;

	case Sftp:
		mData->sftpLoadListing( includeHidden );
		break;

	default:
		throw( QString( "Invalid file protocol!" ) );
	}
}

BaseFile* Location::getFile() {
	return BaseFile::getFile( *this );
}

QString Location::getRemotePath() const {
	return mData->mRemotePath;
}

Location::Protocol Location::getProtocol() const {
	return mData->mProtocol;
}

SshHost* Location::getRemoteHost() const {
	return mData->getHost();
}

void LocationShared::sftpLoadListing( bool includeHidden ) {
	SFTPRequest* request =
		new SFTPRequest( SFTPRequest::Ls,
		                 Callback( this,
		                           SLOT( sshLsSuccess( QVariantMap ) ),
		                           SLOT( sftpLsFailure( QString, int ) ) ) );
	request->setPath( mRemotePath );
	request->setIncludeHidden( includeHidden );
	getHost()->sendSftpRequest( request );
}

void LocationShared::sshLoadListing( bool includeHidden ) {
	QMap< QString, QVariant > params;
	params.insert( "dir", mRemotePath );
	if ( includeHidden ) {
		params.insert( "hidden", true );
	}
	getHost()->sendSlaveRequest( mSudo,
	                             NULL,
	                             "ls",
	                             QVariant( params ),
	                             Callback( this,
	                                       SLOT( sshLsSuccess( QVariantMap ) ),
	                                       SLOT( sshLsFailure( QString, int ) ) ) );
}

void LocationShared::sshLsSuccess( QVariantMap results ) {
	QList< Location > children;
	Location parentLocation( this );

	QVariantMap entries = results.value( "entries" ).toMap();
	QMapIterator< QString, QVariant > i( entries );
	while ( i.hasNext() ) {
		i.next();

		QVariantMap entry = i.value().toMap();
		QByteArray flags = entry.value( "f", "" ).toByteArray();
		bool isDir = flags.contains( 'd' );
		bool canRead = flags.contains( 'r' );
		bool canWrite = flags.contains( 'w' );
		int size = entry.value( "s", 0 ).toInt();
		qint64 lastModified = entry.value( "m", 0 ).toLongLong();

		children.append( Location( parentLocation,
		                           mPath + "/" + i.key(),
		                           isDir ? Location::Directory : Location::File,
		                           size,
		                           QDateTime::fromMSecsSinceEpoch( lastModified * 1000 ),
		                           canRead,
		                           canWrite ) );
	}


	gDispatcher->emitLocationListSuccess( children, mPath );
}

void LocationShared::sshLsFailure( QString error, int flags ) {
	gDispatcher->emitLocationListFailure( error, mPath, flags & SlaveRequest::PermissionError );
}

void LocationShared::sftpLsFailure( QString error, int /*flags*/ ) {
	gDispatcher->emitLocationListFailure( error, mPath, false );
}

SshHost* LocationShared::getHost() {
	if ( mHost == NULL ) {
		mHost = SshHost::getHost( mRemoteHostName.toLatin1(), mRemoteUserName.toLatin1() );
	}

	return mHost;
}

void Location::addToFavorites() {
	if ( ! isNull() ) {
		foreach ( Favorite f, sFavorites ) {
			if ( f.path == getPath() ) {
				return;
			}
		}

		Favorite f;
		f.name = getDefaultFavoriteName();
		f.path = getPath();

		FavoriteLocationDialog dialog( NULL, &f );
		if ( dialog.exec() == QDialog::Accepted ) {
			addSortedFavorite( f );
			saveFavorites();
		}
	}
}

void Location::deleteFavorite( const QString& path ) {
	for ( int i = 0; i < sFavorites.length(); i++ ) {
		if ( sFavorites[i].path == path ) {
			sFavorites.removeAt( i );
			saveFavorites();
			return;
		}
	}
}

void Location::saveFavorites() {
	QSettings settings;

	int index = 0;
	settings.beginWriteArray( ntr( "favorites" ) );
	foreach ( Favorite f, sFavorites ) {
		settings.setArrayIndex( index++ );
		settings.setValue( ntr( "name" ), f.name );
		settings.setValue( ntr( "path" ), f.path );
	}
	settings.endArray();
}

void Location::loadFavorites() {
	QSettings settings;

	int count = settings.beginReadArray( ntr( "favorites" ) );
	for ( int i = 0; i < count; i++ ) {
		settings.setArrayIndex( i );

		Favorite f;
		f.name = settings.value( ntr( "name" ) ).toString();
		f.path = settings.value( ntr( "path" ) ).toString();
		addSortedFavorite( f );
	}
}

void Location::addSortedFavorite( const Favorite& favorite ) {
	int i;
	for ( i = 0; i < sFavorites.length(); i++ ) {
		if ( favorite.name.compare( sFavorites[i].name, Qt::CaseInsensitive ) > 0 ) {
			break;
		}
	}
	sFavorites.insert( i, favorite );
}

QString Location::getDefaultFavoriteName() {
	switch ( mData->mProtocol ) {
	case Ssh:
	case Sftp:
		return QObject::tr( "%1 on %2", "eg: ~ on Server X" ).arg( getLabel() ).arg( mData->mRemoteHostName );

	case Unsaved:
		return QObject::tr( "Unsaved" );

	case Local:
	default:
		return QObject::tr( "%1 (local)" ).arg( getLabel() );
	}
}

void Location::createNewDirectory( const QString& name, const Callback &callback ) {
	switch ( mData->mProtocol ) {
	case Ssh: {
		QVariantMap params;
		params.insert( "dir", mData->mRemotePath + "/" + name );
		mData->getHost()->sendSlaveRequest( mData->mSudo, NULL, "mkdir", QVariant( params ), callback );
		break;
	}

	case Sftp: {
		SFTPRequest* request = new SFTPRequest( SFTPRequest::MkDir, callback );
		request->setPath( mData->mRemotePath + "/" + name );
		mData->getHost()->sendSftpRequest( request );
		break;
	}

	case Local:
		if ( QDir( getPath() ).mkdir( name ) ) {
			callback.triggerSuccess();
		} else {
			callback.triggerFailure( "Unknown error" );
		}
		break;

	default: break;
	}
}

bool Location::canSudo() const {
	return mData->mProtocol == Ssh;
}

Location Location::getSudoLocation() const {
	if ( isSudo() ) {
		return *this;
	}
	return Location( mData->mRemoteUserName + "*@" + mData->mRemoteHostName + ":" + mData->mRemotePath );
}
