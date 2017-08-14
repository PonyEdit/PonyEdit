#ifndef SERVERCHANNEL_H
#define SERVERCHANNEL_H

#include <QByteArray>
#include <QMap>
#include <QSet>
#include "shellchannel.h"

class SshHost;
class ServerFile;
class ServerRequest;
class ServerChannel : public ShellChannel {
	Q_OBJECT

	public:
		explicit ServerChannel( SshHost* host, bool sudo );
		~ServerChannel();
		bool update();

		static void initialize();
		virtual Type getType() {
			return mSudo ? SudoServer : Server;
		}

		inline bool handlesFileBuffer( ServerFile* file ) {
			return mBufferIds.contains( file );
		}

	signals:
		void channelShutdown(); // Used to signal associated ServerFiles.

	protected:
		void shellReady();
		void finalizeServerInit( const QByteArray& initString );

		virtual QByteArray getServerRun( bool sudo );

		virtual bool mainUpdate();

		virtual int getConnectionScore();
		virtual QString getConnectionDescription();

		void criticalError( const QString& error );

	protected:
		enum InternalStatus {
			_WaitingForShell = 40,
			_CheckingServer = 41,
			_CheckingServerResponse = 42,
			_StartingServerUploader = 43,
			_WaitingForServerUploader = 44,
			_UploadingServerScript = 45,
			_WaitingForServerUploadResponse = 46,
			_SendingSudoPassword = 47,
			_WaitingForRequests = 48,
			_SendingRequest = 49
		};

		bool handleOpening();
		void setInternalStatus( InternalStatus newStatus );

		InternalStatus mInternalStatus;
		ServerRequest* mCurrentRequest;
		int mNextMessageId;

		bool mSudo;
		QByteArray mSudoPasswordAttempt;
		bool mTriedSudoPassword;

		QMap< int, ServerRequest* > mRequestsAwaitingReplies;
		QMap< ServerFile*, int > mBufferIds;

		static QByteArray sServerScript;

		static QByteArray sServerChannelInit;
		static QByteArray sServerUpload;
};

#endif  // SERVERCHANNEL_H
