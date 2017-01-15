#include "sshsession.h"
#include "sshhost.h"
#include "sshchannel.h"
#include "passworddlg.h"
#include "dialogrethreader.h"
#include "main/ponyedit.h"
#include "main/globaldispatcher.h"
#include "hostkeydlg.h"
#include "main/tools.h"

#include <QDebug>
#include <libssh2.h>
#include <openssl/opensslconf.h>
#include <openssl/crypto.h>
#include <QSocketNotifier>
#include "QsLog.h"

#ifdef Q_OS_WIN
	#include <windows.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
	#define closesocket close
#endif

#define KEEPALIVE_MSEC (15 * 60 * 1000)
#define TIMEOUT_MSEC (20 * 60 * 1000)

//	TODO: Check for an OpenSSL / LibSSH2 code that needs to be called on shutdown

bool SshSession::sLibsInitialized;
QMap<int, QMutex*> SshSession::sSslMutexes;

SshSession::SshSession(SshHost* host) :
    mHost(host),
    mStatus(Disconnected),
    mErrorDetails(),
    mThreadEndedCalled(false),
    mKeepaliveSent(),
    mLastActivityTimer(),
    mThread(new SshSessionThread(this)),
    mSocket(0),
    mHandle(0),
    mSocketReadNotifier(0),
    mSocketExceptionNotifier(0),
    mChannels(),
    mChannelsLock(),
    mAtChannelLimit(false),
    mPasswordAttempt()
{
	SSHLOG_TRACE(host) << "Opening a new session";

	initializeLibrary();

	QObject::connect(this, SIGNAL(killThread()), mThread, SLOT(quit()), Qt::QueuedConnection);
	moveToThread(mThread);	//	Give this QObject to the new thread; then all signals received by this object are run in the thread.

	QObject::connect(mThread, SIGNAL(finished()), this, SLOT(threadEnded()));
}

SshSession::~SshSession()
{
	//	Send a signal to tell the thread to die...
	emit killThread();

	//	Wait for it to be dead, with a timeout.
	if (!mThread->wait(2000))
	{
		QLOG_WARN() << "SshSession timed out during shutdown";
	}

	delete mThread;
	delete mSocketReadNotifier;
	delete mSocketExceptionNotifier;
}

void SshSession::threadEnded()
{
	//	Ensure this method is only called once. OSX has a habit of calling it twice.
	if (mThreadEndedCalled) return;
	mThreadEndedCalled = true;

	//	Tell the SshHost that I am no longer available. It will manage deletion of my remaining sessions.
	emit sessionClosed(this);
}

void SshSession::start()
{
	mThread->start();
}

void SshSession::threadMain()
{
	bool holdingLock = false;
	QTimer heartbeatTimer;

	heartbeatTimer.setInterval(1000);
	heartbeatTimer.setSingleShot(false);
	QObject::connect(&heartbeatTimer, SIGNAL(timeout()), this, SLOT(heartbeat()));
	heartbeatTimer.start();

	try
	{
		//	In the interests of UI sanity, only let 1 session per host connect at any given time. That way, passwords and confirmation dialogs won't duplicate.
		mHost->lockNewSessions();
		holdingLock = true;

		//	Connect to the remote server
		connect();

		mHost->unlockNewSessions();
		holdingLock = false;

		//	Kick libssh2 over to non-blocking mode. QSocketNotifiers require non-blocking sockets.
		libssh2_session_set_blocking(mHandle, false);

		//	Hook my socket to Qt's event loop, allowing me to receive signals on network events
		mSocketReadNotifier = new QSocketNotifier(mSocket, QSocketNotifier::Read);
		mSocketExceptionNotifier = new QSocketNotifier(mSocket, QSocketNotifier::Exception);

		QObject::connect(mSocketReadNotifier, SIGNAL(activated(int)), this, SLOT(handleReadActivity()));
		QObject::connect(mSocketExceptionNotifier, SIGNAL(activated(int)), this, SLOT(updateAllChannels()));
		QObject::connect(mHost, SIGNAL(wakeAllSessions()), this, SLOT(updateAllChannels()), Qt::QueuedConnection);

		mSocketReadNotifier->setEnabled(true);
		mSocketExceptionNotifier->setEnabled(true);

		//	Queue up at least one channel update.
		queueChannelUpdate();

		//	Enter Qt's event loop for this thread.
		mThread->exec();

	}
	catch (QString error)
	{
		SSHLOG_ERROR(mHost) << "Unexpected throw in main session loop: " << error;
		setErrorStatus("Thrown error: " + error);
		if (holdingLock) mHost->unlockNewSessions();
	}

	//	Close my socket (if I have one)
	if (mSocket != 0)
	{
		closesocket(mSocket);
		mSocket = 0;
	}
}

void SshSession::handleReadActivity()
{
	resetActivityCounter();
	updateAllChannels();
}

bool SshSession::openSocket(unsigned long ipAddress)
{
	resetActivityCounter();

	setStatus(OpeningConnection);
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(mHost->getPort());
	sin.sin_addr.s_addr = ipAddress;

	SSHLOG_INFO(mHost) << "Connecting to" << Tools::stringifyIpAddress(ipAddress);
	int rc = ::connect(mSocket, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
	if (rc == 0)
	{
		SSHLOG_INFO(mHost) << "Connection established";
		mHost->setCachedIpAddress(ipAddress);
		return true;
	}
	else
	{
		SSHLOG_ERROR(mHost) << "Connection refused - socket error";
		return false;
	}
}

bool SshSession::openSocket()
{
	setStatus(NsLookup);

	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	bool connected = false;

	//	If there is a cached ip address, try that first.
	if (mHost->getCachedIpAddress())
	{
		SSHLOG_TRACE(mHost) << "Atempting to connect using a cached IP address:" << Tools::stringifyIpAddress(mHost->getCachedIpAddress());
		connected = openSocket(mHost->getCachedIpAddress());
	}

	if (!connected)
	{
		//	DNS lookup, cycle through the results until we find a working IP address
		SSHLOG_TRACE(mHost) << "Performing DNS lookup on" << mHost->getHostname();
		struct hostent* server = gethostbyname(mHost->getHostname());
		int tryAddress = 0;

		if (!server)
			throw(tr("Could not find host: %1").arg(QString(mHost->getHostname())));

		while (!connected && server->h_addr_list[tryAddress] != 0)
			connected = openSocket(*(u_long*)server->h_addr_list[tryAddress++]);
	}

	return connected;
}

bool SshSession::verifyHostFingerprint()
{
	setStatus(VerifyingHost);

	QByteArray fingerprint = QByteArray(libssh2_hostkey_hash(mHandle, LIBSSH2_HOSTKEY_HASH_SHA1), 20);
	QByteArray knownFingerprint = mHost->getHostFingerprint();
	if (knownFingerprint != fingerprint)
	{
		QString title;
		QString body;
		if (knownFingerprint.isEmpty())
		{
			title = QObject::tr("%1: Verify Host Fingerprint").arg(mHost->getName());
			body = QObject::tr(
				"There is no fingerprint on file for this host. If this is the first time you are connecting to this server, "
				"it is safe to ignore this warning.\n\nHost fingerprints provide extra security by verifying that you are "
				"connecting to the server that you think you are. The first time you connect to a new server, there will be "
				"no fingerprint on file. You will be warned whenever you connect to a server if the host fingerprint has "
				"changed.");
		}
		else
		{
			title = QObject::tr("%1: Host Fingerprint Changed!").arg(mHost->getName());
			body = QObject::tr(
				"The host's fingerprint does not match the one on file! This may be caused by a security breach, or by "
				"server reconfiguration. Please check with your host administrator before accepting the changed host key!");
		}
		body += QObject::tr("\n\nFingerprint: %1").arg(QString(fingerprint.toHex()));

		QVariantMap options;
		options.insert("title", title);
		options.insert("body", body);

		QVariantMap result = DialogRethreader::rethreadDialog<HostKeyDlg>(options);
		if (!result.value("accepted").toBool())
			return false;

		mHost->setNewFingerprint(fingerprint);
	}

	return true;
}

bool SshSession::authenticatePassword(bool keyboardInteractive)
{
	SSHLOG_TRACE(mHost) << "Authenticating by password" << (keyboardInteractive ? "(kbd interactive)" : "");

	bool authenticated = false;
	bool passwordRejected = false;
	bool rememberPassword = mHost->getSavePassword();
	mPasswordAttempt = mHost->getPassword();

	while (true)
	{
		//	Try authenticating (if we have a password yet)
		if (!mPasswordAttempt.isNull())
		{
			int rc;
			if (keyboardInteractive)
				rc = libssh2_userauth_keyboard_interactive_ex(mHandle, mHost->getUsername(), mHost->getUsername().length(), interactiveAuthCallback);
			else
				rc = libssh2_userauth_password(mHandle, mHost->getUsername(), mPasswordAttempt);

			if (!rc)
			{
				authenticated = true;
				break;
			}

			if (rc != LIBSSH2_ERROR_AUTHENTICATION_FAILED)
				throw(QObject::tr("Connection dropped!"));

			passwordRejected = true;
		}

		//	Authentication failed, or no password yet. Ask the user for a password
		QVariantMap options;
		options.insert("title", QObject::tr("%1 Password").arg(mHost->getName()));
		options.insert("blurb", QObject::tr(passwordRejected ? "Your password for %1 was rejected. Please try again." : "Please enter your password for %1 below.").arg(mHost->getName()));
		options.insert("memorable", true);
		options.insert("remember", rememberPassword);

		QVariantMap result = DialogRethreader::rethreadDialog<PasswordDlg>(options);
		if (!result.value("accepted").toBool())
			break;

		mPasswordAttempt = result.value("password").toByteArray();
		rememberPassword = result.value("remember").toBool();
	}

	if (authenticated)
	{
		SSHLOG_INFO(mHost) << "Password accepted.";
		mHost->setPassword(mPasswordAttempt);
		mHost->setSavePassword(rememberPassword);
		Tools::saveServers();
	}

	mPasswordAttempt.clear();
	return authenticated;
}

void SshSession::resetActivityCounter()
{
	mKeepaliveSent = false;
	mLastActivityTimer.restart();
}

void SshSession::interactiveAuthCallback(const char*, int, const char*, int, int num_prompts, const LIBSSH2_USERAUTH_KBDINT_PROMPT*,
	LIBSSH2_USERAUTH_KBDINT_RESPONSE* responses, void** abstract)
{
	if (num_prompts == 1)
	{
		SshSession* connection = (SshSession*)(*abstract);
#ifdef Q_OS_WIN32
        responses[0].text = _strdup(connection->mPasswordAttempt);
#else
        responses[0].text = strdup(connection->mPasswordAttempt);
#endif
        responses[0].length = connection->mPasswordAttempt.length();
	}
}

bool SshSession::authenticatePublicKey()
{
	SSHLOG_TRACE(mHost) << "Attempting ssh key authentication";

	//	If a keyfile has been supplied, try that first.
	if (!mHost->getKeyFile().isEmpty())
	{		
		QByteArray passphrase = mHost->getKeyPassphrase();
		bool remember = mHost->getSaveKeyPassphrase();
		while (true)
		{
			SSHLOG_TRACE(mHost) << "Trying keyfile at " << mHost->getKeyFile();
			int rc = libssh2_userauth_publickey_fromfile_ex(mHandle, mHost->getUsername(), mHost->getUsername().length(), NULL, mHost->getKeyFile(), passphrase);
			if (rc == 0)
			{
				SSHLOG_INFO(mHost) << "Key file authentication succeded!";
				mHost->setKeyPassphrase(passphrase);
				mHost->setSaveKeyPassphrase(remember);
				Tools::saveServers();
				return true;
			}

			if (rc == LIBSSH2_ERROR_PUBLICKEY_UNVERIFIED || rc == LIBSSH2_ERROR_FILE)
			{
				//	Ask the user for a new key passphrase
				QVariantMap options;
				options.insert("title", QObject::tr("%1 Key Passphrase").arg(mHost->getName()));
				options.insert("blurb", QObject::tr(mHost->getKeyPassphrase().isNull() ? "Please enter your key passphrase below." : "Your key passphrase was rejected. Please try again."));
				options.insert("memorable", true);
				options.insert("remember", remember);

				QVariantMap result = DialogRethreader::rethreadDialog<PasswordDlg>(options);
				if (!result.value("accepted").toBool())
					break;

				passphrase = result.value("password").toByteArray();
				remember = result.value("remember").toBool();
			}
			else
			{
				SSHLOG_ERROR(mHost) << "Key file authentiation rejected";
				break;
			}
		}

		return false;
	}

	//	Otherwise, attempt agent authentication as a backup plan
	return authenticateAgent();
}

bool SshSession::authenticateAgent()
{
	struct libssh2_agent_publickey *identity, *prevIdentity = NULL;
	LIBSSH2_AGENT* agent = NULL;
	int rc;
	bool success = false;

	SSHLOG_TRACE(mHost) << "Looking for an SSH key agent";

	try
	{
		//	Initialize SSH agent code
		agent = libssh2_agent_init(mHandle);
		if (!agent) throw(QObject::tr("Failed to call libssh2_agent_init!"));

		//	Connect to the SSH agent
		rc = libssh2_agent_connect(agent);
		if (rc) throw(QObject::tr("No SSH agent found."));

		//	Prepare a list of identities managed by the SSH agent
		rc = libssh2_agent_list_identities(agent);
		if (rc) throw(QObject::tr("Failed to retrieve identities from SSH agent!"));

		while (1)
		{
			//	Get an identity
			rc = libssh2_agent_get_identity(agent, &identity, prevIdentity);
			if (rc) throw(rc == 1 ? QObject::tr("No identities stored in SSH agent were accepted!") : QObject::tr("Failed to receive identity from SSH agent!"));

			//	Try an identity
			SSHLOG_INFO(mHost) << "Trying identity" << identity->comment << "from SSH agent";
			rc = libssh2_agent_userauth(agent, mHost->getUsername(), identity);
			if (rc)
				SSHLOG_INFO(mHost) << "Identity rejected";
			else
			{
				SSHLOG_INFO(mHost) << "Identity accepted";
				success = true;
				break;
			}

			prevIdentity = identity;
		}
	}
	catch (const QString& error)
	{
		SSHLOG_ERROR(mHost) << "Failed to authenticate via SSH agent: " << error;
	}

	if (agent)
	{
		libssh2_agent_disconnect(agent);
		libssh2_agent_free(agent);
	}

	return success;
}

bool SshSession::authenticate(AuthMethod method)
{
	//	TODO: Add authentication support for key files.

	bool authenticated = false;
	switch (method)
	{
		case AuthPassword:
			authenticated = authenticatePassword(false);
			break;

		case AuthKeyboardInteractive:
			authenticated = authenticatePassword(true);
			break;

		case AuthPublicKey:
			authenticated = authenticatePublicKey();
			break;

		default: break;
	}

	if (authenticated)
		mHost->setCachedAuthMethod(method);

	return authenticated;
}

bool SshSession::authenticate()
{
	setStatus(Authenticating);

	bool authenticated = false;

	//	If there is a cached authentication method, use that...
	AuthMethod cachedMethod = mHost->getCachedAuthMethod();
	if (cachedMethod)
	{
		//	... but not if it's Password/Keyboard-Interactive and there's no password on file.
		if (!mHost->getPassword().isEmpty() || (cachedMethod != AuthPassword && cachedMethod != AuthKeyboardInteractive))
		{
			SSHLOG_TRACE(mHost) << "Attempting to use last successful authentication method: " << mHost->getCachedAuthMethod();
			authenticated = authenticate(mHost->getCachedAuthMethod());
		}
	}

	//	Otherwise, query the server and try them in order of preference.
	if (!authenticated)
	{
		AuthMethods authMethods = getAuthenticationMethods();

		if (authMethods & AuthPublicKey)
			authenticated = authenticate(AuthPublicKey);

		if (!authenticated && (authMethods & AuthPassword))
			authenticated = authenticate(AuthPassword);

		if (!authenticated && (authMethods & AuthKeyboardInteractive))
			authenticated = authenticate(AuthKeyboardInteractive);
	}

	return authenticated;
}

SshSession::AuthMethods SshSession::getAuthenticationMethods()
{
	AuthMethods methods = AuthNone;

	char* authlist = libssh2_userauth_list(mHandle, mHost->getUsername(), mHost->getUsername().length());
	if (NULL == authlist)
		return methods;

	if (strstr(authlist, "password") != NULL)
		methods |= AuthPassword;
	if (strstr(authlist, "keyboard-interactive") != NULL)
		methods |= AuthKeyboardInteractive;
	if (strstr(authlist, "publickey") != NULL)
		methods |= AuthPublicKey;

	return methods;
}

void SshSession::connect()
{
	if (!openSocket())
		throw(QObject::tr("Failed to connect to remote host."));

	//	Create an SSH2 session
	mHandle = libssh2_session_init_ex(NULL, NULL, NULL, (void*)this);
	libssh2_trace(mHandle, LIBSSH2_TRACE_SOCKET|LIBSSH2_TRACE_TRANS|LIBSSH2_TRACE_KEX|LIBSSH2_TRACE_AUTH|LIBSSH2_TRACE_CONN|LIBSSH2_TRACE_SCP|LIBSSH2_TRACE_SFTP|LIBSSH2_TRACE_ERROR|LIBSSH2_TRACE_PUBLICKEY);
	libssh2_session_flag(mHandle, LIBSSH2_FLAG_SIGPIPE, 0);
	if (int rc = libssh2_session_handshake(mHandle, mSocket))
		throw(QObject::tr("Failed to start session: %1").arg(rc));
	libssh2_keepalive_config(mHandle, 1, (KEEPALIVE_MSEC / 1000) - 5);

	//	Verify the host fingerprint
	if (!verifyHostFingerprint())
		throw(QObject::tr("Connection aborted due to unaccepted host fingerprint"));

	if (!authenticate())
		throw(QObject::tr("Failed to authenticate with remote host."));

	setStatus(Connected);
}

void SshSession::initializeLibrary()
{
	if (sLibsInitialized) return;

#ifdef Q_OS_WIN
	//	Initialize winsock
	WSADATA data;
	if (WSAStartup(0x22, &data) != 0)
		throw(QObject::tr("Failed to initialize WinSock!"));
#endif

	//	Prepare OpenSSL for multi-threaded work
	CRYPTO_set_locking_callback(manageSslMutex);
	CRYPTO_THREADID_set_callback(sslThreadId);

	//	Initialize libssh2
	if (libssh2_init(0) != 0)
		throw(QString("Failed to initialize LibSSH2!"));

	sLibsInitialized = true;
}

void SshSession::manageSslMutex(int mode, int n, const char* /*file*/, int /*line*/)	//	Callback for OpenSSL threading support
{
	if (PonyEdit::isApplicationExiting()) return;

	//	Mutexes are kept in a map, id->mutex. First time an id is requested, a new mutex is added to handle it.
	QMutex* mutex = sSslMutexes.value(n, NULL);
	if (!mutex)
	{
		mutex = new QMutex();
		sSslMutexes.insert(n, mutex);
	}

	//	Lock or unlock mutexes, depending on what OpenSSL is asking for.
	if (mode & CRYPTO_LOCK)
		mutex->lock();
	else
		mutex->unlock();
}

void SshSession::sslThreadId(CRYPTO_THREADID* threadId)
{
	// Stop unused parameter warnings
	(void)threadId;
	CRYPTO_THREADID_set_pointer(threadId, QThread::currentThread());
}

bool SshSession::isAtChannelLimit()
{
	return mAtChannelLimit;
}

void SshSession::adoptChannel(SshChannel *channel)
{
	channel->setSession(this);

	mChannelsLock.lock();
	mChannels.append(channel);
	mChannelsLock.unlock();

	queueChannelUpdate();
}

void SshSession::queueChannelUpdate()
{
	QMetaObject::invokeMethod(this, "updateAllChannels", Qt::QueuedConnection);
}

void SshSession::updateAllChannels()
{
	bool goAgain = true;
	while (goAgain)
	{
		goAgain = false;

		mChannelsLock.lock();
		for (int i = 0; i < mChannels.length(); i++)
		{
			//	Update the channel
			SshChannel* channel = mChannels[i];
			if (NULL == channel)
				continue;

			bool doMore;
			try
			{
				doMore = channel->updateChannel();
			}
			catch(QString err)
			{
				QLOG_ERROR() << "Critical channel failure:" << err;
				setErrorStatus(QObject::tr("Critical channel failure: ") + err);
				mThread->quit();		//	Abort QThread::exec, fall back to threadMain for cleanup.
				continue;
			}

			if (doMore)
				goAgain = true;

			//	Deal with any of the possible failure states
			SshChannel::Status status = channel->getStatus();
			switch (status)
			{
			case SshChannel::Sessionless:	//	Server refused to create another channel on this session; pass back to SshHost to find a new home.
				mChannels.removeAt(i--);
				mAtChannelLimit = true;
				mHost->setChannelLimitGuess(mChannels.count());
				emit hitChannelLimit(channel);
				break;

			case SshChannel::Error:	//	Serious read/write error occurred. Assume the whole session is dead.
				setErrorStatus(QObject::tr("Critical channel failure: ") + channel->getErrorDetails());
				mThread->quit();		//	Abort QThread::exec, fall back to threadMain for cleanup.
				break;

			case SshChannel::Disconnected:	//	Neatly disconnected. Just take it out of the roster, notify the host.
				mChannels.removeAt(i--);
				emit channelNeatlyClosed(channel);
				break;

			default:	//	All was ok :)
				break;
			}
		}
		mChannelsLock.unlock();

		//	If this update cycle went by without incident and I have headroom, see if there are homeless channels to adopt.
		SshChannel* homelessChannel;
		while (!isAtChannelLimit() && (homelessChannel = mHost->takeNextHomelessChannel()) != NULL)
			adoptChannel(homelessChannel);
	}
}

void SshSession::setErrorStatus(const QString& error)
{
	mErrorDetails = error;
	setStatus(Error);
}

void SshSession::setStatus(Status newStatus)
{
	if (newStatus != mStatus)
	{
		mStatus = newStatus;
		mHost->invalidateOverallStatus();
	}
}

SshChannel* SshSession::getMostConnectedChannel()
{
	SshChannel* result = NULL;

	mChannelsLock.lock();
	if (mStatus < Connected)
		result = (mChannels.length() ? mChannels.at(0) : NULL);
	else
	{
		int mostConnectedScore = 0;
		foreach (SshChannel* channel, mChannels)
		{
			int connectionScore = channel->getConnectionScore();
			if (connectionScore > mostConnectedScore)
			{
				mostConnectedScore = connectionScore;
				result = channel;
				if (mostConnectedScore == 100) break;
			}
		}
	}
	mChannelsLock.unlock();

	return result;
}

QString SshSession::getConnectionDescription()
{
	switch (mStatus)
	{
	case NsLookup:
		return tr("Finding host");

	case OpeningConnection:
		return tr("Connecting");

	case VerifyingHost:
		return tr("Checking host");

	case Authenticating:
		return tr("Authenticating");

	default:
		return "";
	}
}

void SshSession::heartbeat()
{
	if (mLastActivityTimer.elapsed() > TIMEOUT_MSEC)
	{
		setErrorStatus(QObject::tr("Session timeout"));
		mThread->quit();
	}
	else if (mLastActivityTimer.elapsed() > KEEPALIVE_MSEC)
	{
		SSHLOG_TRACE(mHost) << "Sending keepalive heartbeat.";
		mKeepaliveSent = true;
		int dontCare;
		if (libssh2_keepalive_send(mHandle, &dontCare) != 0)
		{
			setErrorStatus(QObject::tr("Connection ironically died during keepalive"));
			mThread->quit();
		}
	}
}






