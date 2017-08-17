#include <QDebug>
#include "main/globaldispatcher.h"
#include "main/ponyedit.h"
#include "main/tools.h"
#include "serverchannel.h"
#include "serverrequest.h"
#include "sftpchannel.h"
#include "sftprequest.h"
#include "ssh2/serverconfigdlg.h"
#include "sshhost.h"
#include "sshsession.h"
#include "xferchannel.h"
#include "xferrequest.h"

QList< SshHost * > SshHost::sKnownHosts;
QMap< QString, QByteArray > SshHost::sKnownHostFingerprints;

SshHost::SshHost() :
	mDesiredStatus( Disconnected ),
	mFirstServerScriptChecker( 0 ),
	mServerScriptChecked( false ),
	mCachedIpAddress( 0 ),
	mCachedAuthMethod( SshSession::AuthNone ),
	mChannelLimitGuess( CHANNEL_LIMIT_GUESS ),
	mSaveHost( true ),
	mSavePassword( false ),
	mSaveKeyPassphrase( false ),
	mPort( 22 ) {
	mOverallStatusDirty = true;
	updateOverallStatus();

	QObject::connect( this,
	                  SIGNAL( overallStatusInvalidated() ),
	                  this,
	                  SLOT( updateOverallStatus() ),
	                  Qt::QueuedConnection );
}

SshHost::~SshHost() {
	// Kill every session violenty.
	foreach ( SshSession *session, mSessions ) {
		delete session;
	}
}

void SshHost::cleanup() {
	// Call every host's destructor, forcing them to do nasty shutdowns.
	foreach ( SshHost *host, sKnownHosts ) {
		delete host;
	}
}

SshHost *SshHost::getHost( const QByteArray &hostname, const QByteArray &username ) {
	// Search through the list of known hosts to see if we already know it...
	foreach ( SshHost *host, sKnownHosts ) {
		if ( host->getHostname() == hostname && host->getUsername() == username ) {
			return host;
		}
	}

	// No? Time to create a new one.
	SshHost *newHost = new SshHost();
	newHost->setHostname( hostname );
	newHost->setUsername( username );

	ServerConfigDlg dlg;
	dlg.setEditHost( newHost );
	if ( dlg.exec() != QDialog::Accepted ) {
		delete newHost;
		return NULL;
	}

	sKnownHosts.append( newHost );
	Tools::saveServers();
	return newHost;
}

void SshHost::connect() {
	if ( mDesiredStatus == Connected ) {
		return;
	}
	mDesiredStatus = Connected;
}

void SshHost::disconnect() {
	if ( mDesiredStatus == Disconnected ) {
		return;
	}
	mDesiredStatus = Disconnected;
}

SshHost::LogHelper::~LogHelper() {
	QString completeLine = QsLogging::LogMessage( mBuffer, QDateTime::currentDateTime(), mLevel ).formatted;
	QsLogging::Logger::Helper( mLevel ).stream() << completeLine;
	mHost->appendToHostLog( completeLine );
}

void SshHost::appendToHostLog( const QString &line ) {
	mModifyingLogMutex.lock();

	// Don't keep more than 1000 lines of in-memory log per server
	if ( mHostLog.length() >= 1000 ) {
		mHostLog.removeFirst();
	}
	mHostLog.append( line );
	mModifyingLogMutex.unlock();

	emit newLogLine( line );
}

void SshHost::showLog() {
	if ( mLogWindow.isNull() ) {
		mLogWindow = QPointer< HostLog >( new HostLog( this ) );
	}
	mLogWindow.data()->show();
	mLogWindow.data()->setFocus();
}

void SshHost::checkHeadroom() {
	if ( mChannels.length() == 0 ) {
		return;
	}

	// Take a tally of how much free headroom there is for channels
	int freeChannels = 0;
	foreach ( SshSession *session, mSessions ) {
		if ( ! session->isAtChannelLimit() && session->getChannelCount() < mChannelLimitGuess ) {
			freeChannels += mChannelLimitGuess - session->getChannelCount();
		}
	}

	// If there are less than 3 channels, set a new session happening...
	if ( freeChannels < 3 ) {
		openSession();
	}
}

void SshHost::channelRejected( SshChannel *channel ) {
	// re-register the channel.
	assignSession( channel );
}

void SshHost::channelNeatlyClosed( SshChannel *channel ) {
	removeChannel( channel );
}

void SshHost::removeChannel( SshChannel *channel ) {
	switch ( channel->getType() ) {
	case SshChannel::Server:
		failServerRequests( "Channel closed.",
		                    0,
		                    mServerRequestQueueMutex,
		                    mServerRequestQueue,
		                    static_cast< ServerChannel * >( channel ) );
		break;

	case SshChannel::SudoServer:
		failServerRequests( "Channel closed.",
		                    0,
		                    mSudoServerRequestQueueMutex,
		                    mSudoServerRequestQueue,
		                    static_cast< ServerChannel * >( channel ) );
		break;

	case SshChannel::Xfer:
		failXferRequests( "Channel closed.", 0, mXferRequestQueueMutex, mXferRequestQueue );
		break;

	case SshChannel::SudoXfer:
		failXferRequests( "Channel closed.", 0, mSudoXferRequestQueueMutex, mSudoXferRequestQueue );
		break;

	default:
		SSHLOG_WARN( this ) << "RemoveChannel is not handling type " << channel->getType();
	}

	mChannels.removeAll( channel );
}

void SshHost::registerChannel( SshChannel *channel ) {
	// Keep a record.
	mChannels.append( channel );
	assignSession( channel );
}

void SshHost::assignSession( SshChannel *channel ) {
	// Just add this channel to the homeless queue.
	mHomelessChannelsMutex.lock();
	mHomelessChannels.append( channel );
	mHomelessChannelsMutex.unlock();

	// Check that there are enough sessions with free room to adopt the homeless...
	checkHeadroom();

	// Give all sessions a nudge, to make sure this channel gets adopted
	wakeAllSessions();
}

SshChannel *SshHost::takeNextHomelessChannel() {
	SshChannel *taken = NULL;

	if ( PonyEdit::isApplicationExiting() ) {
		return NULL;
	}

	mHomelessChannelsMutex.lock();
	if ( ! mHomelessChannels.isEmpty() ) {
		taken = mHomelessChannels.takeFirst();
	}
	mHomelessChannelsMutex.unlock();

	return taken;
}

SshSession *SshHost::openSession() {
	SshSession *newSession = new SshSession( this );
	QObject::connect( newSession,
	                  SIGNAL( hitChannelLimit( SshChannel * ) ),
	                  this,
	                  SLOT( channelRejected( SshChannel * ) ) );
	QObject::connect( newSession,
	                  SIGNAL( channelNeatlyClosed( SshChannel * ) ),
	                  this,
	                  SLOT( channelNeatlyClosed( SshChannel * ) ) );
	QObject::connect( newSession,
	                  SIGNAL( sessionClosed( SshSession * ) ),
	                  this,
	                  SLOT( sessionEnded( SshSession * ) ) );
	mSessions.append( newSession );
	newSession->start();

	invalidateOverallStatus();

	return newSession;
}

void SshHost::showStatus() {
	SSHLOG_INFO( this ) << mSessions.length() << " sessions";
	int i = 1;
	foreach ( SshSession *session, mSessions ) {
		SSHLOG_INFO( this ) << "Session " << i++ << " has " << session->getChannelCount() << " channels.";
	}
}

void SshHost::sendSftpRequest( SFTPRequest *request ) {
	mSftpRequestQueueMutex.lock();
	mSftpRequestQueue.append( request );
	mSftpRequestQueueMutex.unlock();

	emit wakeAllSessions();
	checkChannelCount();
}

void SshHost::sendServerRequest( bool sudo,
                                 ServerFile *file,
                                 const QByteArray &request,
                                 const QVariant &parameters,
                                 const Callback &callback ) {
	// Special case: If the request is "open", pass NULL as the file to the ServerRequest constructor, and attach
	// the file pointer with setOpeningFile. ServerChannels use the file passed to the constructor to determine if a
	// request must be handled by a particular channel.
	bool openingFile = ( request == "open" );
	ServerRequest *newRequest = new ServerRequest( openingFile ? NULL : file, request, parameters, callback );
	if ( openingFile ) {
		newRequest->setOpeningFile( file );
	}

	// Before even enqueing the request, autofail it if it belongs to a channel that's gone.
	ServerFile *relevantFile = newRequest->getFile();
	if ( relevantFile != NULL ) {
		bool ok = false;
		foreach ( SshChannel *channel, mChannels ) {
			if ( channel->is( SshChannel::Server ) || channel->is( SshChannel::SudoServer ) ) {
				ServerChannel *serverChannel = static_cast< ServerChannel * >( channel );
				if ( serverChannel->handlesFileBuffer( relevantFile ) ) {
					ok = true;
					break;
				}
			}
		}

		if ( ! ok ) {
			newRequest->failRequest( QString( "Channel closed" ), 0 );
			delete newRequest;
			return;
		}
	}

	// Add the new request to the queue.
	if ( sudo ) {
		mSudoServerRequestQueueMutex.lock();
		mSudoServerRequestQueue.append( newRequest );
		mSudoServerRequestQueueMutex.unlock();
	} else {
		mServerRequestQueueMutex.lock();
		mServerRequestQueue.append( newRequest );
		mServerRequestQueueMutex.unlock();
	}

	// Nudge all sessions - if their threads are asleep they need to get this message
	emit wakeAllSessions();

	// Ensure there are enough server channels to handle this.
	checkChannelCount();
}

void SshHost::checkChannelCount() {
	// For each type of channel, check their counts.
	int serverCount = countChannels( SshChannel::Server );
	int sudoServerCount = countChannels( SshChannel::SudoServer );
	int xferCount = countChannels( SshChannel::Xfer );
	int sudoXferCount = countChannels( SshChannel::SudoXfer );
	int sftpCount = countChannels( SshChannel::Sftp );

	// Check if we need more server channels.
	if ( serverCount < MAX_SERVER_CHANNELS &&
	     mServerRequestQueue.length() > serverCount * SERVER_CHANNEL_QUEUE_MULTIPLIER ) {
		registerChannel( new ServerChannel( this, false ) );
	}

	// Check if we need more server channels.
	if ( sudoServerCount < MAX_SERVER_CHANNELS &&
	     mSudoServerRequestQueue.length() > sudoServerCount * SERVER_CHANNEL_QUEUE_MULTIPLIER ) {
		registerChannel( new ServerChannel( this, true ) );
	}

	// Check if we need more server channels.
	if ( xferCount < MAX_XFER_CHANNELS && mXferRequestQueue.length() > xferCount * XFER_CHANNEL_QUEUE_MULTIPLIER ) {
		registerChannel( new XferChannel( this, false ) );
	}

	// Check if we need more server channels.
	if ( sudoXferCount < MAX_XFER_CHANNELS &&
	     mSudoXferRequestQueue.length() > sudoXferCount * XFER_CHANNEL_QUEUE_MULTIPLIER ) {
		registerChannel( new XferChannel( this, true ) );
	}

	// Check if we need more sftp channels.
	if ( sftpCount < MAX_SFTP_CHANNELS && mSftpRequestQueue.length() > sftpCount * SFTP_CHANNEL_QUEUE_MULTIPLIER ) {
		registerChannel( new SFTPChannel( this ) );
	}
}

int SshHost::countChannels( SshChannel::Type type ) {
	int count = 0;
	foreach ( SshChannel *channel, mChannels ) {
		if ( channel->is( type ) ) {
			count++;
		}
	}
	return count;
}

SFTPRequest *SshHost::getNextSftpRequest() {
	SFTPRequest *request = NULL;

	mSftpRequestQueueMutex.lock();
	if ( ! mSftpRequestQueue.isEmpty() ) {
		request = mSftpRequestQueue.takeFirst();
	}
	mSftpRequestQueueMutex.unlock();

	return request;
}

ServerRequest *SshHost::getNextServerRequest( bool sudo, const QMap< ServerFile *, int > &registeredBuffers ) {
	ServerRequest *request = NULL;

	QMutex &lock = sudo ? mSudoServerRequestQueueMutex : mServerRequestQueueMutex;
	QList< ServerRequest * > &list = sudo ? mSudoServerRequestQueue : mServerRequestQueue;

	lock.lock();
	for ( int i = 0; i < list.length(); i++ ) {
		ServerFile *file = list[i]->getFile();
		if ( file == NULL || registeredBuffers.contains( file ) ) {
			// TODO: Add some code to ensure no request gets lost in the queue permanently if its
			// buffer-locked channel gets killed
			request = list.takeAt( i );
			break;
		}
	}
	lock.unlock();

	return request;
}

bool SshHost::waitBeforeCheckingServer( SshChannel *channel ) {
	// Only wait if the server script hasn't been checked before.
	if ( mServerScriptChecked ) {
		return false;
	}

	// Don't need to wait if you *are* the very first channel through the gate
	if ( mFirstServerScriptChecker == channel ) {
		return false;
	}

	bool result = true;
	mFirstServerScriptCheckerLock.lock();
	if ( mFirstServerScriptChecker == NULL ) {
		mFirstServerScriptChecker = channel;
		result = false;
	}
	mFirstServerScriptCheckerLock.unlock();

	return result;
}

void SshHost::firstServerCheckComplete() {
	mServerScriptChecked = true;
	mFirstServerScriptChecker = NULL;
}

void SshHost::handleUnsolicitedServerMessage( const QVariantMap & /*message*/ ) {
	SSHLOG_INFO( this ) << "Unsolicited server message received";
}

void SshHost::getFileContent( bool sudo, const QByteArray &filename, const Callback &callback ) {
	enqueueXferRequest( new XferRequest( sudo, filename, callback ) );
}

void SshHost::setFileContent( bool sudo,
                              const QByteArray &filename,
                              const QByteArray &content,
                              const Callback &callback ) {
	XferRequest *rq = new XferRequest( sudo, filename, callback );
	rq->setData( content );
	rq->setDataSize( content.length() );
	rq->setUpload( true );
	enqueueXferRequest( rq );
}

void SshHost::enqueueXferRequest( XferRequest *request ) {
	if ( request->isSudo() ) {
		mSudoXferRequestQueueMutex.lock();
		mSudoXferRequestQueue.append( request );
		mSudoXferRequestQueueMutex.unlock();
	} else {
		mXferRequestQueueMutex.lock();
		mXferRequestQueue.append( request );
		mXferRequestQueueMutex.unlock();
	}

	// Nudge all sessions - if their threads are asleep they need to get this message
	emit wakeAllSessions();

	checkChannelCount();
}

XferRequest *SshHost::getNextXferRequest( bool sudo ) {
	XferRequest *request = NULL;

	if ( sudo ) {
		mSudoXferRequestQueueMutex.lock();
		if ( mSudoXferRequestQueue.length() ) {
			request = mSudoXferRequestQueue.takeFirst();
		}
		mSudoXferRequestQueueMutex.unlock();
	} else {
		mXferRequestQueueMutex.lock();
		if ( mXferRequestQueue.length() ) {
			request = mXferRequestQueue.takeFirst();
		}
		mXferRequestQueueMutex.unlock();
	}

	return request;
}

const QByteArray &SshHost::getHomeDirectory() {
	if ( mHomeDirectory.isNull() ) {
		mHomeDirectory = "/home/" + mUsername;
	}

	return mHomeDirectory;
}

SshHost *SshHost::getBlankHost( bool save ) {
	SshHost *host = new SshHost();
	if ( ! host ) {
		return NULL;
	}

	host->setSaveHost( save );
	sKnownHosts.append( host );

	gDispatcher->emitSshServersUpdated();
	return host;
}

QString SshHost::getDefaultPath() {
	if ( mConnectionType == SFTP ) {
		return QString( "sftp://" ) + ( mUsername.isEmpty() ? QString( "" ) : mUsername + QString( "@" ) ) +
		       mHostname + QString( "/" ) + mDefaultDirectory;
	} else {
		return ( mUsername.isEmpty() ? "" : mUsername + "@" ) + mHostname + ":" + mDefaultDirectory;
	}
}

Location SshHost::getDefaultLocation() {
	return Location( getDefaultPath() );
}

void SshHost::updateOverallStatus() {
	if ( ! mOverallStatusDirty ) {
		return;
	}

	// Work out which channel is closest to being connected, and display that channel's status.
	SshChannel *mostConnectedChannel = NULL;
	SshSession *mostConnectedSession = NULL;
	int mostConnectedScore = 0;

	foreach ( SshSession *session, mSessions ) {
		SshChannel *localMaximum = session->getMostConnectedChannel();
		int score = localMaximum ? localMaximum->getConnectionScore() : session->getStatus();
		if ( score > mostConnectedScore ) {
			mostConnectedScore = score;
			mostConnectedSession = session;
			mostConnectedChannel = localMaximum;
			if ( score == 100 ) {
				break;
			}
		}
	}

	if ( mostConnectedScore == 100 ) {
		setOverallStatus( Connected, tr( "Connected" ) );
	} else if ( mostConnectedChannel ) {
		setOverallStatus( Connecting, mostConnectedChannel->getConnectionDescription() );
	} else if ( mostConnectedSession ) {
		setOverallStatus( Connecting, mostConnectedSession->getConnectionDescription() );
	} else {
		setOverallStatus( Disconnected, tr( "Disconnected" ) );
	}

	mOverallStatusDirty = false;
}

void SshHost::setOverallStatus( Status newStatus, const QString &connectionString ) {
	if ( newStatus != mOverallStatus || connectionString != mConnectionString ) {
		mConnectionString = connectionString;
		mOverallStatus = newStatus;

		emit overallStatusChanged();
	}
}

void SshHost::invalidateOverallStatus() {
	mOverallStatusDirty = true;
	emit overallStatusInvalidated();
}

void SshHost::sessionEnded( SshSession *session ) {
	// By the time, we get in here, the session's thread should be terminated and the connection closed.
	// Remove the session, and all its channels from my records.
	mSessions.removeAll( session );
	foreach ( SshChannel *channel, session->getChannels() ) {
		removeChannel( channel );
	}

	// Something special to watch for: If this was the last session, fail all queued requests and channels
	if ( mSessions.length() == 0 ) {
		failAllHomelessChannels();
		failAllRequests( tr( "Failed to (re)connect to remote host!" ), ServerRequest::ConnectionError );
	}

	// Delete all channels, and the session. This is a separate step from removing channels from my records,
	// because deleting channels triggers reconnect attempts; I want my records clear before reconnect attempts.
	foreach ( SshChannel *channel, session->getChannels() ) {
		delete channel;
	}
	delete session;

	checkChannelCount();
	checkHeadroom();
	invalidateOverallStatus();
}

void SshHost::failAllHomelessChannels() {
	mHomelessChannelsMutex.lock();
	foreach ( SshChannel *channel, mHomelessChannels ) {
		mChannels.removeAll( channel );
		delete channel;
	}
	mHomelessChannels.clear();
	mHomelessChannelsMutex.unlock();
}

void SshHost::failAllRequests( const QString &error, int flags ) {
	failServerRequests( error, flags, mServerRequestQueueMutex, mServerRequestQueue, NULL );
	failServerRequests( error, flags, mSudoServerRequestQueueMutex, mSudoServerRequestQueue, NULL );
	failXferRequests( error, flags, mXferRequestQueueMutex, mXferRequestQueue );
	failXferRequests( error, flags, mSudoXferRequestQueueMutex, mSudoXferRequestQueue );
}

void SshHost::failServerRequests( const QString &error,
                                  int flags,
                                  QMutex &listLock,
                                  QList< ServerRequest * > &requestList,
                                  ServerChannel *channel ) {
	listLock.lock();
	QList< ServerRequest * > listCopy = requestList;
	requestList.clear();
	listLock.unlock();

	foreach ( ServerRequest *request, listCopy ) {
		ServerFile *relatedFile = request->getFile();
		if ( channel == NULL || channel->handlesFileBuffer( relatedFile ) ) {
			request->failRequest( error, flags );
			delete request;
		}
	}
}

void SshHost::failXferRequests( const QString &error,
                                int flags,
                                QMutex &listLock,
                                QList< XferRequest * > &requestList ) {
	listLock.lock();
	for ( int i = 0; i < requestList.length(); i++ ) {
		XferRequest *request = requestList[i];
		requestList.removeAt( i );
		request->handleFailure( error, flags );
		delete request;
	}
	listLock.unlock();
}

QByteArray SshHost::getHostFingerprint() {
	return sKnownHostFingerprints.value( mHostname );
}

void SshHost::setNewFingerprint( const QByteArray &fingerprint ) {
	sKnownHostFingerprints.insert( mHostname, fingerprint );
	Tools::saveHostFingerprints();
}

void SshHost::registerKnownFingerprint( const QString &hostname, const QByteArray &fingerprint ) {
	sKnownHostFingerprints.insert( hostname, fingerprint );
}

void SshHost::setSaveHost( bool saveHost ) {
	mSaveHost = saveHost;
}

void SshHost::setSavePassword( bool savePassword ) {
	mSavePassword = savePassword;
}

void SshHost::setSaveKeyPassphrase( bool saveKeyPassphrase ) {
	mSaveKeyPassphrase = saveKeyPassphrase;
}
