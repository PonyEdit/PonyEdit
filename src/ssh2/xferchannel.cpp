#include <libssh2.h>
#include <QCryptographicHash>
#include <QDebug>
#include "main/tools.h"
#include "sshhost.h"
#include "xferchannel.h"
#include "xferrequest.h"

XferChannel::XferChannel( SshHost *host, bool sudo ) :
	ServerChannel( host, sudo ),
	mInternalStatus( _WaitingForRequests ),
	mCurrentRequest( nullptr ),
	mBinaryReadBuffer(),
	mLeftoverEscape( false ) {}

bool XferChannel::mainUpdate() {
	if ( mInternalStatus == _WaitingForRequests ) {
		mCurrentRequest = mHost->getNextXferRequest( mSudo );
		if ( mCurrentRequest == nullptr ) {
			return false;
		} else {
			mInternalStatus = _SendingRequestHeader;
			return true;
		}
	}

	if ( mInternalStatus == _SendingRequestHeader ) {
		// If uploading, make sure the data to be uploaded is encoded.
		if ( mCurrentRequest->isUploadRequest() && mCurrentRequest->getEncodedData().isEmpty() ) {
			mCurrentRequest->setEncodedData( Tools::bin( mCurrentRequest->getData() ) );
		}

		SendResponse r = sendData( mCurrentRequest->getRequestHeader() );
		if ( r != SendSucceed ) {
			return ( r == SendAgain );
		}

		mInternalStatus = ( mCurrentRequest->isUploadRequest() ? _WaitingForReady : _ReadingDownloadHeader );
	}

	if ( mInternalStatus == _ReadingDownloadHeader ) {
		ReadReply r = readUntil( "\n" );
		if ( r.readAgain ) {
			return false;
		}
		if ( r.data.isEmpty() ) {
			throw( tr( "Invalid response to download header" ) );
		}

		if ( r.data.startsWith( "Error: " ) ) {
			if ( r.data.contains( "File not found" ) ) {
				mCurrentRequest->handleFailure( QString( mCurrentRequest->getFilename() ) + " - " +
				                                tr( "File not found" ),
				                                0 );
				mInternalStatus = _WaitingForRequests;
				return true;
			} else {
				throw( QString( r.data ) );
			}
		}

		if ( r.data.endsWith( '\r' ) ) {
			SSHLOG_WARN( mHost ) << "Warning: Stray carriage-return after xfer header";
			r.data.chop( 1 );
		}

		QList< QByteArray > parts = r.data.split( ',' );
		mCurrentRequest->setDataSize( parts[ 0 ].toInt() );
		mCurrentRequest->setChecksum( parts[ 1 ] );

		mLeftoverEscape = false;
		mInternalStatus = _DownloadingBody;
	}

	if ( mInternalStatus == _DownloadingBody ) {
		ReadReply r = readBinaryData( mCurrentRequest->getDataSize() );
		if ( r.readAgain ) {
			return false;
		}

		// Check the checksum.
		QCryptographicHash hash( QCryptographicHash::Md5 );
		hash.addData( r.data );
		QByteArray checksum = hash.result().toHex().toLower();

		if ( checksum != mCurrentRequest->getChecksum() ) {
			mCurrentRequest->handleFailure( tr( "Checksum failure: %1 vs %2" ).arg( QString(
													checksum ) ).arg(
								QString( mCurrentRequest->getChecksum() ) ),
			                                0 );
		} else {
			mCurrentRequest->setData( r.data );
			mCurrentRequest->handleSuccess();
		}

		mInternalStatus = _WaitingForOk;
	}

	if ( mInternalStatus == _WaitingForReady ) {
		ReadReply r = readUntil( "\n" );
		if ( r.readAgain ) {
			return true;
		}

		if ( r.data != "Ready" ) {
			criticalError( "Failed to upload file" );
		}

		mInternalStatus = _UploadingBody;
	}

	if ( mInternalStatus == _UploadingBody ) {
		QByteArray encoded = mCurrentRequest->getEncodedData();

		SendResponse r = sendData( encoded );
		if ( r != SendSucceed ) {
			return ( r == SendAgain );
		}

		mCurrentRequest->handleSuccess();

		mInternalStatus = _WaitingForOk;
	}

	if ( mInternalStatus == _WaitingForOk ) {
		ReadReply r = readUntil( "\n" );
		if ( r.readAgain ) {
			return true;
		}

		if ( r.data.endsWith( '\r' ) ) {
			SSHLOG_WARN( mHost ) << "Warning: Stray carriage-return after xfer OK";
			r.data.chop( 1 );
		}

		if ( r.data != "OK" ) {
			criticalError( "Did not receive OK at the end of transmission. Got: " + r.data );
		}

		mInternalStatus = _WaitingForRequests;
		return true;
	}

	return false;
}

ShellChannel::ReadReply XferChannel::readBinaryData( int size ) {
	ReadReply reply;
	reply.readAgain = false;

	if ( size <= 0 ) {
		return reply;
	}

	while ( mBinaryReadBuffer.length() < size ) {
		// Flush anything from the read buffer into the binary buffer
		int transferred = Tools::unbin( mBinaryReadBuffer,
		                                mReadBuffer,
		                                size,
		                                mReadBuffer.length(),
		                                &mLeftoverEscape );
		mReadBuffer = mReadBuffer.right( mReadBuffer.size() - transferred );

		if ( mBinaryReadBuffer.length() == size ) {
			break;
		}

		// See if there's data waiting to be read, push it into the read buffer (raw)
		int rc = libssh2_channel_read( mHandle, mScratchBuffer, SSH_SHELL_BUFFER_SIZE );
		if ( rc > 0 ) {
			mReadBuffer.append( mScratchBuffer, rc );
		} else if ( rc == LIBSSH2_ERROR_EAGAIN ||
		            ( rc == -1 &&
		              libssh2_session_last_errno( mSession->sessionHandle() ) == LIBSSH2_ERROR_EAGAIN ) ) {
			reply.readAgain = true;
			return reply;
		} else {
			criticalError( "Connection closed unexpectedly!" );
		}

		int currentLength = mBinaryReadBuffer.length();
		int percent = static_cast< int >( ( static_cast< float >( currentLength ) / size ) * 100 );
		mCurrentRequest->handleProgress( percent );
	}

	// If it reaches here, enough data has been read.
	reply.data = mBinaryReadBuffer;
	mBinaryReadBuffer.clear();
	return reply;
}

QByteArray XferChannel::getServerRun( bool sudo ) {
	if ( sudo ) {
		mSudoPasswordAttempt = mHost->getSudoPassword();
	}
	return sudo ? "sudo -p Sudo-prompt%-ponyedit-% perl .ponyedit/server.pl xfer" : "perl .ponyedit/server.pl xfer";
}
