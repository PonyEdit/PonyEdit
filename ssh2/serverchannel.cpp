#include <libssh2.h>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>

#include "dialogrethreader.h"
#include "file/serverfile.h"
#include "main/tools.h"
#include "serverchannel.h"
#include "serverrequest.h"
#include "sshhost.h"

#define SERVER_INIT      " cd ~;" \
	"if type perl >/dev/null 2>&1;then " \
	"perl -e '" \
	"use Digest::MD5;" \
	"my $f=\".ponyedit/server.pl\";" \
	"die \"No Server\\n\" if(!-e $f);" \
	"$d=Digest::MD5->new;" \
	"open F,$f;" \
	"$d->addfile(F);" \
	"die \"Server checksum fail\\n\" if($d->hexdigest ne \"[[CHECKSUM]]\");" \
	"exec \"[[SERVER_RUN]]\";" \
	"';" \
	"else " \
	"echo 'No Perl';" \
	"fi\n"


#define SERVER_START_UPLOAD      " perl -e '" \
	"mkdir \".ponyedit\" unless(-d \".ponyedit\");" \
	"open F,\">.ponyedit/server.pl\" or die \"Server write error\\n\";" \
	"print \"uploader ready\";" \
	"while(1)" \
	"{" \
	"my $line=<STDIN>;" \
	"last if($line=~/^END_OF_SERVER/);" \
	"print F $line;" \
	"}" \
	"close F;" \
	"'&&" \
	"[[SERVER_RUN]]\n"

QByteArray ServerChannel::sServerChannelInit( SERVER_INIT );
QByteArray ServerChannel::sServerScript;
QByteArray ServerChannel::sServerUpload;

ServerChannel::ServerChannel( SshHost* host, bool sudo ) :
	ShellChannel( host ),
	mInternalStatus( _WaitingForShell ),
	mCurrentRequest( 0 ),
	mNextMessageId( 1 ),
	mSudo( sudo ),
	mSudoPasswordAttempt(),
	mTriedSudoPassword( false ),
	mRequestsAwaitingReplies(),
	mBufferIds() {
	SSHLOG_TRACE( host ) << "Creating a new server channel";
}

ServerChannel::~ServerChannel() {
	SSHLOG_TRACE( mHost ) << "Server channel deleted";

	// Inform the files that depend on this server channel before passing the critical error
	emit channelShutdown();
}

#include <QTime>

void ServerChannel::initialize() {
	QFile f( Tools::getResourcePath( "server/server.pl" ) );
	if ( ! f.open( QFile::ReadOnly ) ) {
		throw( tr( "Unable to find server script!" ) );
	}
	sServerScript = f.readAll();

	QCryptographicHash hash( QCryptographicHash::Md5 );
	hash.addData( sServerScript );
	QByteArray checksum = hash.result().toHex().toLower();

	sServerChannelInit.replace( "[[CHECKSUM]]", checksum );
	sServerUpload = SERVER_START_UPLOAD;
}

bool ServerChannel::update() {
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

void ServerChannel::shellReady() {
	setInternalStatus( _CheckingServer );
}

bool ServerChannel::handleOpening() {
	// Pass control to the shell channel until it's ready.
	if ( mInternalStatus == _WaitingForShell ) {
		return ShellChannel::handleOpening();
	}

	if ( mInternalStatus == _CheckingServer ) {
		if ( mHost->waitBeforeCheckingServer( this ) ) {
			return true;
		}

		QByteArray serverInit = sServerChannelInit;
		serverInit.replace( "[[SERVER_RUN]]", getServerRun( mSudo ) );

		SendResponse r = sendData( serverInit );
		if ( r == SendAgain ) {
			return true;
		}
		if ( r != SendSucceed ) {
			criticalError( "Failed to send server script initializer" );
		}

		setInternalStatus( _CheckingServerResponse );
	}

	if ( mInternalStatus == _CheckingServerResponse ) {
		ReadReply reply = readUntilPrompt();
		if ( reply.readAgain ) {
			return true;
		}

		if ( reply.data.isNull() ) {
			return false;
		}

		QByteArray response = reply.data.trimmed();
		if ( response.length() == 0 ) {
			return true;    // Try again
		}
		if ( response.startsWith( "No Perl" ) ) {
			criticalError( "No Perl found on the remote server!" );
			return false;
		}

		if ( response.startsWith( "Old Perl" ) ) {
			criticalError( "PonyEdit requires at least Perl 5.8. Found: " + response.mid( 9 ) );
			return false;
		}

		if ( response.startsWith( "Server OK" ) || response.contains( "Sudo-prompt" ) ) {
			finalizeServerInit( response );
			return true;
		}

		if ( response.startsWith( "No Server" ) || response.startsWith( "Server checksum fail" ) ) {
			setInternalStatus( _StartingServerUploader );
		} else {
			criticalError( "Unexpected response to server initialization" );
			return false;
		}
	}

	if ( mInternalStatus == _StartingServerUploader ) {
		QByteArray serverUpload = sServerUpload;
		serverUpload.replace( "[[SERVER_RUN]]", getServerRun( false ) );

		SendResponse r = sendData( serverUpload );
		if ( r == SendAgain ) {
			return true;
		}
		if ( r != SendSucceed ) {
			criticalError( "Failed to start server uploader" );
		}

		setInternalStatus( _WaitingForServerUploader );
	}

	if ( mInternalStatus == _WaitingForServerUploader ) {
		ReadReply rr = readUntil( "uploader ready" );
		if ( rr.readAgain ) {
			return true;
		}

		setInternalStatus( _UploadingServerScript );
	}

	if ( mInternalStatus == _UploadingServerScript ) {
		SendResponse r = sendData( sServerScript + QByteArray( "END_OF_SERVER\n" ) );
		if ( r == SendAgain ) {
			return true;
		}
		if ( r != SendSucceed ) {
			criticalError( "Failed to upload server script" );
		}

		setInternalStatus( _WaitingForServerUploadResponse );
	}

	if ( mInternalStatus == _WaitingForServerUploadResponse ) {
		ReadReply reply = readUntilPrompt();
		if ( reply.readAgain ) {
			return true;
		}

		if ( reply.data.isNull() ) {
			return false;
		}

		QByteArray response = reply.data.trimmed();
		if ( response.length() == 0 ) {
			return true;    // Try again
		}
		if ( response.contains( "Sudo fail" ) ) {
			criticalError( "Failed to sudo" );
			return false;
		}

		if ( response.startsWith( "Server OK" ) || response.contains( "Sudo-prompt" ) ) {
			finalizeServerInit( response );
			return true;
		} else {
			criticalError( "Unexpected response to server initialization: " + response );
			return false;
		}
	}

	if ( mInternalStatus == _SendingSudoPassword ) {
		if ( mTriedSudoPassword ) {
			// Need to ask the user for a new sudo password
			QVariantMap options;
			options.insert( "title", QObject::tr( "%1 Sudo Password" ).arg( mHost->getName() ) );
			options.insert( "blurb",
			                QObject::tr( "Please enter your sudo password for %1 below." ).arg( mHost->
			                                                                                    getName() ) );
			options.insert( "memorable", false );

			QVariantMap result = DialogRethreader::rethreadDialog< PasswordDlg >( options );
			if ( ! result.value( "accepted" ).toBool() ) {
				criticalError( "Failed to sudo" );
				return false;
			}

			mSudoPasswordAttempt = result.value( "password" ).toByteArray();
			mTriedSudoPassword = false;
		}

		// Send the password
		SendResponse r = sendData( mSudoPasswordAttempt + "\n" );
		if ( r == SendAgain ) {
			return true;
		}
		if ( r != SendSucceed ) {
			criticalError( "Failed to send sudo password" );
		}

		mTriedSudoPassword = true;
		mHost->setSudoPassword( mSudoPasswordAttempt );
		setInternalStatus( _WaitingForServerUploadResponse );
	}

	return true;
}

void ServerChannel::criticalError( const QString& error ) {
	// Fail the current job (if there is one)
	if ( mCurrentRequest ) {
		mCurrentRequest->failRequest( error, ServerRequest::ConnectionError );
		delete mCurrentRequest;
		mCurrentRequest = NULL;
	}

	SshChannel::criticalError( error );
}

bool ServerChannel::mainUpdate() {
	// Read as much as there is to be read
	ReadReply rr;
	do {
		rr = readUntil( "\n" );
		if ( ! rr.readAgain ) {
			if ( rr.data.length() ) {
				SSHLOG_TRACE( mHost ) << "Received: " << rr.data;

				// We have a message! Decode it.
				QVariantMap response = QJsonDocument::fromJson( rr.data ).object().toVariantMap();
				if ( int responseId = response.value( "i", 0 ).toInt() ) {
					// Look up the request that this response relates to
					ServerRequest* request = mRequestsAwaitingReplies.value( responseId, NULL );
					if ( request != NULL ) {
						// If the request was opening a file, and a bufferId is returned, record
						// the relationship
						BaseFile::deletionLock();
						{
							if ( request->getOpeningFile() &&
							     response.contains( "bufferId" ) ) {
								mBufferIds.insert( request->getOpeningFile(),
								                   response.value( "bufferId" ).toInt() );
								connect( this,
								         SIGNAL( channelShutdown() ),
								         request->getOpeningFile(),
								         SLOT( serverChannelFailure() ),
								         Qt::QueuedConnection );
							}
						}
						BaseFile::deletionUnlock();

						// Handle the response.
						ServerRequest* request =
							mRequestsAwaitingReplies.value( responseId, NULL );
						if ( request != NULL ) {
							request->handleReply( response );
						}
					} else {
						// This response has an id, but no related request. This should not
						// happen.
						SSHLOG_ERROR( mHost ) <<
						        "Received server response, found no corresponding request";
					}
				} else {
					// Got an unsolicted message. Tell the host.
					mHost->handleUnsolicitedServerMessage( response );
				}
			}
		}
	} while ( ! rr.readAgain && mStatus == Open );

	// If attempting to read resulted in a catastrophic failure, abandon ship!
	if ( mStatus != Open ) {
		return false;
	}

	// Check if there's requests to be made
	if ( mInternalStatus == _WaitingForRequests ) {
		mCurrentRequest = mHost->getNextServerRequest( mSudo, mBufferIds );
		if ( mCurrentRequest ) {
			mCurrentRequest->setMessageId( mNextMessageId++ );
			mInternalStatus = _SendingRequest;

			// If this new request is closing a file, remove it from my record of bufferIds.
			if ( mCurrentRequest->getRequest() == "close" ) {
				disconnect( mCurrentRequest->getFile() );
				mBufferIds.remove( mCurrentRequest->getFile() );
			}
		} else {
			return false;
		}
	}

	// Make requests if there are any to make.
	if ( mInternalStatus == _SendingRequest ) {
		const QByteArray& packedRequest =
			mCurrentRequest->getPackedRequest( mBufferIds.value( mCurrentRequest->getFile(), -1 ) );
		int rc = libssh2_channel_write( mHandle, packedRequest, packedRequest.length() );
		if ( rc < 0 ) {
			if ( rc == -1 ) {
				rc = libssh2_session_last_errno( mSession->sessionHandle() );
			}
			if ( rc == LIBSSH2_ERROR_EAGAIN ) {
				return true;
			}
			criticalError( tr( "Failed to initialize send a server request: %1" ).arg( rc ) );
			return false;
		}

		SSHLOG_TRACE( mHost ) << "Sent: " << packedRequest;
		mRequestsAwaitingReplies.insert( mCurrentRequest->getMessageId(), mCurrentRequest );

		mCurrentRequest = NULL;
		setInternalStatus( _WaitingForRequests );
	}

	return true;
}

void ServerChannel::finalizeServerInit( const QByteArray& initString ) {
	if ( initString.contains( "Sudo-prompt" ) ) {
		mSudoPasswordAttempt = mHost->getSudoPassword();
		mTriedSudoPassword = mSudoPasswordAttempt.isNull();

		mInternalStatus = _SendingSudoPassword;
		return;
	}

	QList< QByteArray > lines = initString.split( '\n' );

	if ( lines.length() < 2 ) {
		criticalError( "Invalid server script init" );
	}

	QJsonParseError error;
	QVariantMap initBlob = QJsonDocument::fromJson( lines[1], &error ).object().toVariantMap();
	if ( error.error ) {
		criticalError( "JSON initialization blob invalid" );
	}

	QByteArray homeDir = initBlob.value( "~" ).toByteArray();
	SSHLOG_INFO( mHost ) << "Home directory: " << homeDir;
	mHost->setHomeDirectory( homeDir );

	setInternalStatus( _WaitingForRequests );

	mHost->firstServerCheckComplete();
	setStatus( Open );
}

int ServerChannel::getConnectionScore() {
	if ( mStatus == Opening ) {
		return mInternalStatus;
	} else {
		return ShellChannel::getConnectionScore();
	}
}

QString ServerChannel::getConnectionDescription() {
	if ( mStatus == Opening ) {
		switch ( mInternalStatus ) {
		case _CheckingServer:
		case _CheckingServerResponse:
			return tr( "Checking server" );

		case _StartingServerUploader:
		case _WaitingForServerUploader:
		case _UploadingServerScript:
		case _WaitingForServerUploadResponse:
			return tr( "Updating server" );

		case _SendingSudoPassword:
			return tr( "Requesting sudo" );

		default:;
		}
	}

	return ShellChannel::getConnectionDescription();
}

void ServerChannel::setInternalStatus( InternalStatus newStatus ) {
	if ( newStatus != mInternalStatus ) {
		mInternalStatus = newStatus;
		mHost->invalidateOverallStatus();
	}
}

QByteArray ServerChannel::getServerRun( bool sudo ) {
	if ( sudo ) {
		mSudoPasswordAttempt = mHost->getSudoPassword();
	}
	return sudo ? "sudo -p Sudo-prompt%-ponyedit-% perl .ponyedit/server.pl" : "perl .ponyedit/server.pl";
}
