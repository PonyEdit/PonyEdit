#include <QCryptographicHash>
#include <QDebug>
#include <QMessageBox>
#include <utility>

#include "file/openfilemanager.h"
#include "file/serverfile.h"
#include "filestatuswidget.h"
#include "main/dialogwrapper.h"
#include "main/globaldispatcher.h"
#include "main/mainwindow.h"
#include "main/tools.h"
#include "ssh2/serverrequest.h"
#include "ssh2/sshhost.h"

ServerFile::ServerFile( const Location &location ) :
	BaseFile( location ) {
	mHost = location.getRemoteHost();
	mChangePumpCursor = 0;
}

ServerFile::~ServerFile() {
	foreach ( Change *change, mChangesSinceLastSave ) {
		delete change;
	}
	mChangesSinceLastSave.clear();
}

BaseFile *ServerFile::newFile( const QString &content ) {
	SSHLOG_TRACE( mHost ) << "Creating a new server file" << content.length() << "bytes long";

	setOpenStatus( BaseFile::Loading );
	clearTempOpenData();

	// Retrieve the file content on a separate SSH channel...
	mHost->setFileContent( mLocation.isSudo(),
	                       mLocation.getRemotePath().toLatin1(),
	                       content.toUtf8(),
	                       Callback( this,
	                                 SLOT( createSuccess( QVariantMap ) ),
	                                 SLOT( openFailure( QString, int ) ),
	                                 SLOT( downloadProgress( int ) ) ) );

	return this;
}

void ServerFile::createSuccess( const QVariantMap & /* response */ ) {
	SSHLOG_TRACE( mHost ) << "Remote file creation successful.";

	open();
	gMainWindow->openSingleFile( mLocation );
}

void ServerFile::open() {
	setOpenStatus( BaseFile::Loading );
	clearTempOpenData();

	// Tell the Server script to open the file as a buffer
	QMap< QString, QVariant > params;
	params.insert( "file", mLocation.getRemotePath() );
	mHost->sendServerRequest( mLocation.isSudo(),
	                          this,
	                          "open",
	                          QVariant( params ),
	                          Callback( this,
	                                    SLOT( serverOpenSuccess( QVariantMap ) ),
	                                    SLOT( openFailure( QString, int ) ) ) );

	// Retrieve the file content on a separate SSH channel...
	mHost->getFileContent( mLocation.isSudo(),
	                       mLocation.getRemotePath().toLatin1(),
	                       Callback( this,
	                                 SLOT( downloadSuccess( QVariantMap ) ),
	                                 SLOT( openFailure( QString, int ) ),
	                                 SLOT( downloadProgress( int ) ) ) );
}

void ServerFile::downloadProgress( int percent ) {
	setProgress( percent );
}

void ServerFile::serverOpenSuccess( QVariantMap results ) {
	if ( getOpenStatus() != BaseFile::Loading ) {
		// Something went wrong while d/ling the file content, but the server open worked :-/
		// TODO: Tell the server to close this file; it's useless to me now.
		clearTempOpenData();
		return;
	}

	mServerOpenResults = std::move( results );
	finalizeFileOpen();
}

void ServerFile::downloadSuccess( const QVariantMap &result ) {
	if ( getOpenStatus() != BaseFile::Loading ) {
		// Something went wrong while asking the server to open the file, but d/l worked :-/
		clearTempOpenData();
		return;
	}

	mDownloadedData = result.value( "data" ).toByteArray();
	mDownloadedChecksum = result.value( "checksum" ).toByteArray();
	finalizeFileOpen();
}

void ServerFile::finalizeFileOpen() {
	// Don't finalize file open until both the server has opened it, and the file is downloaded.
	// Check mDownloadedChecksum instead of mDownloadedData, as the latter will be null for an empty file.
	if ( mServerOpenResults.isEmpty() || mDownloadedChecksum.isNull() ) {
		return;
	}

	// Ensure the checksum from the server matches the checksum from the download
	if ( mServerOpenResults.value( "checksum" ).toByteArray() != mDownloadedChecksum ) {
		openFailure( "Checksum error: Server script disagrees with the downloaded copy", 0 );
		clearTempOpenData();
		return;
	}

	bool readOnly = ! mServerOpenResults.value( "writable" ).toBool();
	BaseFile::openSuccess( QString::fromUtf8( mDownloadedData ), mDownloadedChecksum, readOnly );
	mChangePumpCursor = 0;

	clearTempOpenData();
}

void ServerFile::serverChannelFailure() {
	SSHLOG_ERROR( mHost ) << "File" << mLocation.getLabel() <<
	        "is initiating a reconnection after server channel failure";
	reconnect();
}

void ServerFile::reconnect() {
	setOpenStatus( Reconnecting );
	clearTempOpenData();

	// Tell the server to open the file as a buffer
	QMap< QString, QVariant > params;
	params.insert( "file", mLocation.getRemotePath() );
	mHost->sendServerRequest( mLocation.isSudo(),
	                          this,
	                          "open",
	                          QVariant( params ),
	                          Callback( this,
	                                    SLOT( serverReconnectSuccess( QVariantMap ) ),
	                                    SLOT( serverReconnectFailure( QString, int ) ) ) );
}

void ServerFile::serverReconnectSuccess( const QVariantMap &results ) {
	// Great! Make sure the returned checksum matches what we expect the file to contain.
	if ( results.value( "checksum" ).toByteArray() != mLastSaveChecksum ) {
		// Going to have to fully re-upload the file.
	} else {
		// Just need to resume pumping updates since the last save.
		movePumpCursor( mLastSavedRevision );
		setOpenStatus( Ready );
		pumpChangeQueue();
	}
}

void ServerFile::serverReconnectFailure( const QString &error, int flags ) {
	SSHLOG_ERROR( mHost ) << "Failed to reconnect to remote server for file" << mLocation.getLabel();

	// TODO: Special case permission errors into SUDO system.
	// If this is a connection issue, try again. Else, report the error to the user.
	if ( flags & ServerRequest::ConnectionError ) {
		reconnect();
	} else {
		gDispatcher->emitGeneralErrorMessage(
			mLocation.getLabel() + " has encountered a serious error, and cannot be recovered. " +
			"Please save the file under a different name or on a different host, close it and try reconnecting. Error: " +
			error );
	}
}

void ServerFile::movePumpCursor( int revision ) {
	mChangePumpCursor = 0;
	while ( mChangePumpCursor < mChangesSinceLastSave.length() &&
	        mChangesSinceLastSave[ mChangePumpCursor ]->revision <= revision ) {
		mChangePumpCursor++;
	}
}

void ServerFile::handleDocumentChange( int position, int removeChars, const QString &insert ) {
	if ( mIgnoreChanges ) {
		return;
	}

	BaseFile::handleDocumentChange( position, removeChars, insert );

	auto *change = new Change();
	change->revision = mRevision;
	change->position = position;
	change->remove = removeChars;
	change->insert = insert;
	mChangesSinceLastSave.append( change );

	if ( mOpenStatus == Ready ) {
		pumpChangeQueue();
	}
}

void ServerFile::pumpChangeQueue() {
	while ( mChangePumpCursor < mChangesSinceLastSave.length() ) {
		Change *change = mChangesSinceLastSave[ mChangePumpCursor++ ];

		// Send this change to the remote server
		QMap< QString, QVariant > params;
		params.insert( "p", change->position );
		if ( change->remove > 0 ) {
			params.insert( "d", change->remove );
		}
		if ( change->insert.length() > 0 ) {
			params.insert( "a", change->insert );
		}

		mHost->sendServerRequest( mLocation.isSudo(),
		                          this,
		                          "change",
		                          QVariant( params ),
		                          Callback( this, nullptr, SLOT( changePushFailure( QString, int ) ) ) );
	}
}

void ServerFile::changePushFailure( const QString &error, int /*flags*/ ) {
	SSHLOG_ERROR( mHost ) << "Server push error for file" << mLocation.getLabel() << ": " << error;
}

void ServerFile::save() {
	QMap< QString, QVariant > params;
	QCryptographicHash hash( QCryptographicHash::Md5 );
	hash.addData( mContent.toUtf8() );
	params.insert( "checksum", hash.result().toHex().toLower() );
	params.insert( "revision", mRevision );
	params.insert( "undoLength", mDocument->availableUndoSteps() );

	mHost->sendServerRequest( mLocation.isSudo(),
	                          this,
	                          "save",
	                          QVariant( params ),
	                          Callback( this,
	                                    SLOT( serverSaveSuccess( QVariantMap ) ),
	                                    SLOT( serverSaveFailure( QString, int ) ) ) );
}

void ServerFile::serverSaveSuccess( const QVariantMap &results ) {
	int revision = results.value( "revision" ).toInt();
	int undoLength = results.value( "undoLength" ).toInt();
	QByteArray checksum = results.value( "checksum" ).toByteArray();

	savedRevision( revision, undoLength, checksum );
}

void ServerFile::serverSaveFailure( const QString &error, int flags ) {
	saveFailure( error, flags & ServerRequest::PermissionError );
}

void ServerFile::setLastSavedRevision( int lastSavedRevision ) {
	BaseFile::setLastSavedRevision( lastSavedRevision );

	// Purge all stored changes up to that point...
	while ( mChangesSinceLastSave.length() > 0 && mChangesSinceLastSave[ 0 ]->revision <= mLastSavedRevision ) {
		Change *change = mChangesSinceLastSave.takeFirst();
		mChangePumpCursor--;
		delete change;
	}

	if ( mChangePumpCursor < 0 ) {
		mChangePumpCursor = 0;
	}
}

void ServerFile::close() {
	setOpenStatus( Closing );

	// Just send and assume success.
	mHost->sendServerRequest( mLocation.isSudo(), this, "close" );
	closeCompleted();
}

void ServerFile::refresh() {
	mHost->sendServerRequest( mLocation.isSudo(), this, "close" );
	open();
}

void ServerFile::sudo() {
	// Close the old buffer. Don't bother checking if it succeeds.
	mHost->sendServerRequest( false, this, "close" );
	gOpenFileManager.deregisterFile( this );

	// Change location
	mLocation = mLocation.getSudoLocation();
	mHost = mLocation.getRemoteHost();

	// Reconnect like this was a dropout
	reconnect();
}
