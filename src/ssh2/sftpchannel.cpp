#include <libssh2.h>
#include <libssh2_sftp.h>
#include <QDebug>
#include "file/basefile.h"
#include "serverrequest.h"
#include "sftpchannel.h"
#include "sftprequest.h"
#include "sshhost.h"
#include "sshsession.h"

SFTPChannel::SFTPChannel( SshHost *host ) :
	SshChannel( host ),
	mHandle( NULL ),
	mOperationHandle( NULL ),
	mCurrentRequest( NULL ),
	mRequestState(),
	mResult(),
	mOperationSize( 0 ),
	mOperationCursor( 0 ) {}

bool SFTPChannel::update() {
	switch ( mStatus ) {
		case Opening:
			return handleOpening();

		case Open:
			return mainUpdate();

		case Disconnected:
			return false;

		default:;
	}

	return false;
}

bool SFTPChannel::handleOpening() {
	mHandle = libssh2_sftp_init( mSession->sessionHandle() );
	if ( mHandle == NULL ) {
		int rc = libssh2_session_last_errno( mSession->sessionHandle() );
		if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			return true;
		} else {
			if ( rc == LIBSSH2_ERROR_CHANNEL_FAILURE ) {
				// Reassign this channel elsewhere
				setSession( NULL );
			} else {
				criticalError( tr( "Failed to open a channel %1: %2" ).arg( reinterpret_cast< unsigned long >( mHandle ),
				                                                            0,
				                                                            16 ).arg( rc ) );
			}
			return false;
		}
	}

	setStatus( Open );
	return true;
}

void SFTPChannel::criticalError( const QString &error ) {
	// Fail the current job (if there is one)
	if ( mCurrentRequest ) {
		mCurrentRequest->triggerFailure( error, ServerRequest::ConnectionError );
		delete mCurrentRequest;
		mCurrentRequest = NULL;
	}

	SshChannel::criticalError( error );
}

bool SFTPChannel::mainUpdate() {
	// Make sure there is a request to be handled...
	if ( mCurrentRequest == NULL ) {
		mCurrentRequest = mHost->getNextSftpRequest();
		if ( mCurrentRequest == NULL ) {
			return false;   // No requests in the queue, go to sleep.
		}
		mRequestState = Beginning;
	}

	// Handle the current request...
	bool continueRequest;
	switch ( mCurrentRequest->getType() ) {
		case SFTPRequest::Ls:
			continueRequest = updateLs();
			break;

		case SFTPRequest::ReadFile:
			continueRequest = updateReadFile();
			break;

		case SFTPRequest::WriteFile:
			continueRequest = updateWriteFile();
			break;

		case SFTPRequest::MkDir:
			continueRequest = updateMkDir();
			break;

		default:
			continueRequest = false;
	}

	// If the request is finished, delete it.
	if ( ! continueRequest ) {
		delete mCurrentRequest;
		mCurrentRequest = NULL;
	}

	return true;    // Even if request finished, come back to check queue.
}

bool SFTPChannel::updateLs() {
	int rc;

	if ( mRequestState == Beginning ) {
		mOperationHandle = libssh2_sftp_opendir( mHandle, mCurrentRequest->getPath().toUtf8() );
		if ( mOperationHandle ) {
			mRequestState = Reading;
		} else if ( ( rc = libssh2_session_last_errno( mSession->sessionHandle() ) ) == LIBSSH2_ERROR_EAGAIN ) {
			return true;    // try again
		} else {
			criticalError( tr( "Failed to open remote directory for reading: %1" ).arg( rc ) );
			return false;
		}

		mResult.clear();
	}

	if ( mRequestState == Reading ) {
		char buffer[ 1024 ];
		LIBSSH2_SFTP_ATTRIBUTES attrs;

		rc = libssh2_sftp_readdir( mOperationHandle, buffer, sizeof( buffer ), &attrs );
		if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			return true;    // Try again
		} else if ( rc == 0 ) {
			mRequestState = Finishing;
		} else if ( rc < 0 ) {
			criticalError( tr( "Error while reading remote directory: %1" ).arg( rc ) );
			return false;
		} else {// Got a directory entry

			// Skip hidden entries iff request says to
			if ( mCurrentRequest->getIncludeHidden() || buffer[ 0 ] != '.' ) {
				// Can't determine if remote file is readable/writable, so report all as so.
				QString flags = "rw";
				if ( LIBSSH2_SFTP_S_ISDIR( attrs.permissions ) ) {
					flags += 'd';
				}

				QVariantMap details;
				details.insert( "f", flags );
				details.insert( "s", attrs.filesize );
				details.insert( "m", static_cast< qulonglong >( attrs.mtime ) );
				mResult.insert( buffer, details );
			}
		}
	}

	if ( mRequestState == Finishing ) {
		rc = libssh2_sftp_closedir( mOperationHandle );
		if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			return true;
		} else if ( rc < 0 ) {
			criticalError( tr( "Failed to cleanly close SFTP directory: %1" ).arg( rc ) );
			return false;
		}

		// Success! Send a response and finish up.
		QVariantMap finalResult;
		finalResult.insert( "entries", mResult );
		mCurrentRequest->triggerSuccess( finalResult );
		return false;
	}

	return true;
}

bool SFTPChannel::updateMkDir() {
	QByteArray path = mCurrentRequest->getPath().toUtf8();
	int rc = libssh2_sftp_mkdir_ex( mHandle, path, path.length(), 0644 );
	if ( rc == LIBSSH2_ERROR_EAGAIN ) {
		return true;    // Try again.
	} else if ( rc < 0 ) {
		mCurrentRequest->triggerFailure( tr( "Failed to create remote directory: %1" ).arg( rc ), 0 );
	} else {
		mCurrentRequest->triggerSuccess( QVariantMap() );
	}
	return false;
}

bool SFTPChannel::updateReadFile() {
	int rc;

	if ( mRequestState == Beginning ) {
		QByteArray path = mCurrentRequest->getPath().toUtf8();
		mOperationHandle = libssh2_sftp_open_ex( mHandle,
		                                         path,
		                                         path.length(),
		                                         LIBSSH2_FXF_READ,
		                                         0,
		                                         LIBSSH2_SFTP_OPENFILE );
		if ( mOperationHandle ) {
			mRequestState = Sizing;
		} else if ( ( rc = libssh2_session_last_errno( mSession->sessionHandle() ) ) == LIBSSH2_ERROR_EAGAIN ) {
			return true;    // try again
		} else {
			criticalError( tr( "Failed to open remote file for reading: %1" ).arg( rc ) );
			return false;
		}
	}

	if ( mRequestState == Sizing ) {
		LIBSSH2_SFTP_ATTRIBUTES attr;
		int rc = libssh2_sftp_fstat_ex( mOperationHandle, &attr, 0 );
		if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			return true;
		} else if ( rc < 0 ) {
			criticalError( tr( "Failed to stat remote file: %1" ).arg( rc ) );
			return false;
		}

		mOperationSize = attr.filesize;
		if ( mOperationSize == 0 ) {
			mOperationSize = 1;
		}
		mRequestState = Reading;
	}

	if ( mRequestState == Reading ) {
		char buffer[ 4096 ];

		rc = libssh2_sftp_read( mOperationHandle, buffer, sizeof( buffer ) );
		if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			return true;    // Try again
		} else if ( rc == 0 ) {
			mRequestState = Finishing;
		} else if ( rc < 0 ) {
			criticalError( tr( "Error while reading file contents: %1" ).arg( rc ) );
			return false;
		} else {// Got some data
			mCurrentRequest->addContent( buffer, rc );
			mCurrentRequest->triggerProgress(
				( mCurrentRequest->getContent().length() * 100 ) / mOperationSize );
		}
	}

	if ( mRequestState == Finishing ) {
		rc = libssh2_sftp_close_handle( mOperationHandle );
		if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			return true;
		} else if ( rc < 0 ) {
			criticalError( tr( "Failed to cleanly close SFTP file: %1" ).arg( rc ) );
			return false;
		}

		// Success! Send a response and finish up.
		QVariantMap finalResult;
		finalResult.insert( "content", mCurrentRequest->getContent() );
		mCurrentRequest->triggerSuccess( finalResult );
		return false;
	}

	return true;
}

bool SFTPChannel::updateWriteFile() {
	int rc;

	if ( mRequestState == Beginning ) {
		QByteArray path = mCurrentRequest->getPath().toUtf8();
		mOperationHandle = libssh2_sftp_open_ex( mHandle,
		                                         path,
		                                         path.length(),
		                                         LIBSSH2_FXF_WRITE | LIBSSH2_FXF_TRUNC | LIBSSH2_FXF_CREAT,
		                                         0644,
		                                         LIBSSH2_SFTP_OPENFILE );
		if ( mOperationHandle ) {
			mRequestState = Writing;
		} else if ( ( rc = libssh2_session_last_errno( mSession->sessionHandle() ) ) == LIBSSH2_ERROR_EAGAIN ) {
			return true;    // try again
		} else {
			criticalError( tr( "Failed to open remote file for writing: %1" ).arg( rc ) );
			return false;
		}

		mOperationCursor = 0;
	}

	if ( mRequestState == Writing ) {
		const QByteArray &content = mCurrentRequest->getContent();
		rc = libssh2_sftp_write( mOperationHandle,
		                         content.constData() + mOperationCursor,
		                         content.length() - mOperationCursor );
		if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			return true;    // Try again
		} else if ( rc < 0 ) {
			criticalError( tr( "Error while writing file contents: %1" ).arg( rc ) );
			return false;
		} else {// Wrote some data
			mOperationCursor += rc;
			mCurrentRequest->triggerProgress( ( mOperationCursor * 100 ) / content.length() );
		}

		if ( mOperationCursor >= content.length() ) {
			mRequestState = Finishing;
		}
	}

	if ( mRequestState == Finishing ) {
		rc = libssh2_sftp_close_handle( mOperationHandle );
		if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			return true;
		} else if ( rc < 0 ) {
			criticalError( tr( "Failed to cleanly close SFTP file: %1" ).arg( rc ) );
			return false;
		}

		// Success! Send a response and finish up.
		QVariantMap finalResult;
		finalResult.insert( "revision", mCurrentRequest->getRevision() );
		finalResult.insert( "undoLength", mCurrentRequest->getUndoLength() );
		finalResult.insert( "checksum", BaseFile::getChecksum( mCurrentRequest->getContent() ) );
		mCurrentRequest->triggerSuccess( finalResult );
		return false;
	}

	return true;
}
