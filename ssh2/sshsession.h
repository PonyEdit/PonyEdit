#ifndef SSHSESSION_H
#define SSHSESSION_H

#include <QMap>
#include <QMutex>
#include <QThread>
#include <QTime>
#include <QWaitCondition>

#include <libssh/callbacks.h>
#include <libssh/libssh.h>

//
// Note: All SshSession objects should only be created or deleted in the main thread,
// by SshHost..!
//

class SshHost;
class SshChannel;
class SshSessionThread; // Defined at the bottom of this file.
class QSocketNotifier;

class SshSession : public QObject {
	friend class SshSessionThread;
	Q_OBJECT

	public:
		enum Status { Error = -1, Disconnected = 0, NsLookup = 1, OpeningConnection = 2, VerifyingHost = 3, Authenticating = 4, Connected = 100 };
		enum AuthMethod { AuthNone = 0, AuthPassword = 1, AuthKeyboardInteractive = 2, AuthPublicKey = 4 };
		Q_DECLARE_FLAGS( AuthMethods, AuthMethod )

		SshSession( SshHost* host );
		~SshSession();

		void start();

		bool isAtChannelLimit();// Returns true if this session has hit the channel limit, and no channels have
		                        // been closed.
		inline int getChannelCount() const {
			return mChannels.length();
		}

		inline const QList< SshChannel* >& getChannels() const {
			return mChannels;
		}

		SshChannel* getMostConnectedChannel();  // Returns the SshChannel closest to completing its connection.

		inline ssh_session sessionHandle() const {
			return mHandle;
		}

		inline Status getStatus() const {
			return mStatus;
		}

		QString getConnectionDescription();

	signals:
		void hitChannelLimit( SshChannel* rejectedChannel );
		void channelNeatlyClosed( SshChannel* channel );
		void sessionClosed( SshSession* session );

		// Used interally:
		void killThread();

	protected slots:
		void handleReadActivity();
		void updateAllChannels();
		void threadEnded();
		void heartbeat();

	protected:
		void adoptChannel( SshChannel* channel );
		void threadMain();

		void queueChannelUpdate();

		void connect();
		void setStatus( Status newStatus );
		void setErrorStatus( const QString& error );

		bool openSocket();
		bool openSocket( unsigned long ipAddress );

		bool verifyHostFingerprint();

		bool authenticate();
		bool authenticate( AuthMethod method );
		bool authenticatePassword( bool keyboardInteractive );
		bool authenticatePublicKey();
		bool authenticateAgent();
		AuthMethods getAuthenticationMethods();
		/*static void interactiveAuthCallback( const char*,
		 *                                    int,
		 *                                    const char*,
		 *                                    int,
		 *                                    int,
		 *                                    const LIBSSH2_USERAUTH_KBDINT_PROMPT*,
		 *                                    LIBSSH2_USERAUTH_KBDINT_RESPONSE*,
		 *                                    void** );
		 */

		static void initializeLibrary();

		static int sshMutexInit( void ** ) {
			return 0;
		};
		static int sshMutexDestroy( void ** ) {
			return 0;
		};

		static int sshMutexLock( void ** );
		static int sshMutexUnlock( void ** );
		static unsigned long sshMutexThreadId();

		struct ssh_threads_callbacks_struct* getSshCallbacks() {
			return &sshThreadsCallbacks;
		}

		void resetActivityCounter();

	private:
		static struct ssh_threads_callbacks_struct sshThreadsCallbacks;

		SshHost* mHost;
		Status mStatus;
		QString mErrorDetails;

		bool mThreadEndedCalled;

		bool mKeepaliveSent;
		QTime mLastActivityTimer;

		SshSessionThread* mThread;
		int mSocket;
		ssh_session mHandle;
		QSocketNotifier* mSocketReadNotifier;
		QSocketNotifier* mSocketExceptionNotifier;

		static bool sLibsInitialized;
		static QMap< int, QMutex* > sSslMutexes;

		QList< SshChannel* > mChannels;
		QMutex mChannelsLock;
		bool mAtChannelLimit;

		QByteArray mPasswordAttempt;    // Used to pass the current password attempt to the interactive keyboard
		                                // callback
};

Q_DECLARE_OPERATORS_FOR_FLAGS( SshSession::AuthMethods )

class SshSessionThread : public QThread {
	public:
		SshSessionThread( SshSession* session ) :
			mSession( session ) {}
		void run() {
			mSession->threadMain();
		}

		int exec() {
			return QThread::exec();
		}

	private:
		SshSession* mSession;
};

#endif  // SSHSESSION_H
