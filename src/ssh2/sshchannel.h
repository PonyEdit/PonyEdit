#ifndef SSHCHANNEL_H
#define SSHCHANNEL_H

#include <QObject>

//
// SshChannel objects are always created and destroyed in the main thread by SshHost,
// but are used for most of their life by SshChannel in the SshChannels' thread.
//

class SshHost;
class SshSession;
class SshChannel : public QObject {
	Q_OBJECT

	public:
		enum Status {
			Error             = -1,
			Disconnected      = 0,
			Sessionless       = 10,
			WaitingForSession = 11,
			Opening           = 12,
			Open              = 100
		};
		enum Type {
			Shell      = 0x0001,
			Server     = 0x0002,
			Xfer       = 0x0003,
			Sftp       = 0x0004,
			Sudo       = 0x8000,
			SudoServer = Server | Sudo,
			SudoXfer   = Xfer | Sudo
		};

		explicit SshChannel( SshHost *host );   // Only construct new SshChannel objects inside SshHost!!
		~SshChannel();  // Only delete SshChannel objects inside SshHost!!

		bool updateChannel();   // Calls 'update' to do any actual work.
		virtual bool update()  = 0;     // Return true if more to be done immediately.
		virtual Type getType() = 0;
		inline bool is( Type type ) {
			return getType() == type;
		}

		void setSession( SshSession *session );
		inline Status getStatus() const {
			return mStatus;
		}

		inline const QString &getErrorDetails() const {
			return mErrorDetails;
		}

		virtual void criticalError( const QString &error );

		virtual int getConnectionScore();       // Returns a number indicative of how close to connected this
		                                        // channel
		                                        // is.
		virtual QString getConnectionDescription();

	protected:
		void setStatus( Status status );

		SshHost *mHost;
		SshSession *mSession;

		Status mStatus;
		QString mErrorDetails;
};

#endif  // SSHCHANNEL_H
