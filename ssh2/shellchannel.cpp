#include <libssh2.h>
#include <QDebug>
#include "shellchannel.h"
#include "sshhost.h"
#include "sshsession.h"

#define MACHINE_READABLE_INIT " stty -echo; export PS1=\\%-ponyedit-\\%\n"
#define MACHINE_READABLE_PROMPT "%-ponyedit-%"

ShellChannel::ShellChannel( SshHost* host, bool machineReadable, const QByteArray& ptyType ) :
	SshChannel( host ),
	mHandle( NULL ),
	mReadBuffer(),
	mScratchBuffer(),
	mInternalStatus( _OpenSession ),
	mMachineReadable( machineReadable ),
	mPtyType( ptyType ) {}

bool ShellChannel::update() {
	switch ( mStatus ) {
	case Opening:
		return handleOpening();

	case Open:
		return mainUpdate();

	default:;
	}

	return false;
}

bool ShellChannel::handleOpening() {
	int rc;

	if ( mInternalStatus == _OpenSession ) {
		mHandle = libssh2_channel_open_session( mSession->sessionHandle() );
		if ( mHandle == NULL ) {
			int rc = libssh2_session_last_errno( mSession->sessionHandle() );
			if ( rc == LIBSSH2_ERROR_EAGAIN ) {
				return true;
			} else {
				if ( rc == LIBSSH2_ERROR_CHANNEL_FAILURE ) {
					// This channel needs to be given away; this connection can't handle it.
					// TODO: Detect if this channel request has been handed off too often and kill
					// it if so.
					setSession( NULL );
				} else {
					criticalError( tr( "Failed to open a channel %1: %2" ).arg( ( unsigned long )
					                                                            mHandle,
					                                                            0,
					                                                            16 ).arg( rc ) );
				}
				return false;
			}
		}
		setInternalStatus( _RequestPty );
	}

	if ( mInternalStatus == _RequestPty ) {
		// Due to a bug in OpenSSH (popular server-side implementation of SSH), we must simulate blocking for
		// PTY request.
		while ( ( rc = libssh2_channel_request_pty( mHandle, mPtyType ) ) == LIBSSH2_ERROR_EAGAIN ) {}
		;
		if ( rc < 0 ) {
			criticalError( tr( "Failed to request an appropriate pty %1: %2" ).arg( ( unsigned long )
			                                                                        mHandle,
			                                                                        0,
			                                                                        16 ).arg(
					       rc ) );
			return false;
		}

		setInternalStatus( _StartShell );
	}

	if ( mInternalStatus == _StartShell ) {
		if ( mMachineReadable ) {
			rc = libssh2_channel_exec( mHandle, "sh" );
		} else {
			rc = libssh2_channel_shell( mHandle );
		}
		if ( rc < 0 ) {
			if ( rc == -1 ) {
				rc = libssh2_session_last_errno( mSession->sessionHandle() );
			}
			if ( rc == LIBSSH2_ERROR_EAGAIN ) {
				return true;
			}
			criticalError( tr( "Failed to open a shell %1: %2" ).arg( ( unsigned long ) mHandle,
			                                                          0,
			                                                          16 ).arg( rc ) );
			return false;
		}

		if ( mMachineReadable ) {
			setInternalStatus( _SendInit );
		} else {
			shellReady();
		}
	}

	if ( mInternalStatus == _SendInit ) {
		SendResponse r = sendData( MACHINE_READABLE_INIT );
		if ( r == SendAgain ) {
			return true;
		}
		if ( r != SendSucceed ) {
			criticalError( "Failed to send shell initializer" );
		}

		setInternalStatus( _WaitForInitReply );
	}

	if ( mInternalStatus == _WaitForInitReply ) {
		ReadReply reply = readUntilPrompt();
		if ( reply.readAgain ) {
			return true;
		}
		if ( reply.data.isNull() ) {
			return false;
		}

		shellReady();
	}

	return true;
}

bool ShellChannel::mainUpdate() {
	return false;
}

ShellChannel::ReadReply ShellChannel::readUntilPrompt() {
	return readUntil( MACHINE_READABLE_PROMPT );
}

ShellChannel::ReadReply ShellChannel::readUntil( const QByteArray& marker ) {
	ReadReply result;
	result.readAgain = false;

	// Check if the buffer is already loaded and ready to go.
	int markerIndex = mReadBuffer.indexOf( marker );
	if ( markerIndex == -1 ) {
		// Actually do the read
		int rc = libssh2_channel_read( mHandle, mScratchBuffer, SSH_SHELL_BUFFER_SIZE );
		if ( rc > 0 ) {
			mReadBuffer.append( mScratchBuffer, rc );
		} else if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			result.readAgain = true;
			return result;
		} else if ( rc == 0 ) {
			if ( libssh2_channel_eof( mHandle ) ) {
				criticalError( "Connection closed unexpectedly" );
			}
		} else {
			criticalError( "Failed to read from host" );
		}

		// Check again to see if the buffer is ready to go.
		markerIndex = mReadBuffer.indexOf( marker );
	}

	// If there's data ready to go, pack it into the reply.
	if ( markerIndex > -1 ) {
		result.data = mReadBuffer.left( markerIndex );
		mReadBuffer = mReadBuffer.right( mReadBuffer.size() - ( markerIndex + marker.length() ) );
	}

	return result;
}

void ShellChannel::shellReady() {
	setStatus( Open );
}

ShellChannel::SendResponse ShellChannel::sendData( const QByteArray &data ) {
	int rc = libssh2_channel_write( mHandle, data, data.length() );
	if ( rc < 0 ) {
		if ( rc == -1 ) {
			rc = libssh2_session_last_errno( mSession->sessionHandle() );
		}
		if ( rc == LIBSSH2_ERROR_EAGAIN ) {
			return SendAgain;
		}
		criticalError( tr( "Failed to initialize send a server request: %1" ).arg( rc ) );
		return SendFail;
	}

	return SendSucceed;
}

int ShellChannel::getConnectionScore() {
	if ( mStatus == Opening ) {
		return mInternalStatus;
	} else {
		return SshChannel::getConnectionScore();
	}
}

QString ShellChannel::getConnectionDescription() {
	if ( mStatus == Opening ) {
		switch ( mInternalStatus ) {
		case _OpenSession:
			return tr( "Opening channel" );

		case _RequestPty:
			return tr( "Requesting PTY" );

		case _StartShell:
		case _SendInit:
		case _WaitForInitReply:
			return tr( "Preparing shell" );
		}
	}

	return SshChannel::getConnectionDescription();
}

void ShellChannel::setInternalStatus( InternalStatus newStatus ) {
	if ( newStatus != mInternalStatus ) {
		mInternalStatus = newStatus;
		mHost->invalidateOverallStatus();
	}
}
