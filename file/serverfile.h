#ifndef SSHFILE_H
#define SSHFILE_H

#include "basefile.h"

class OldServerChannel;
class RemoteConnection;
class ServerFile : public BaseFile {
	Q_OBJECT

	public:
		BaseFile *newFile( const QString &content );
		void open();
		void save();
		void close();
		void refresh();

		ServerFile( const Location &location );  // Do not call; use File::getFile instead

		virtual void sudo();

	public slots:
		void createSuccess( QVariantMap result );

		void serverOpenSuccess( QVariantMap results );
		void downloadProgress( int percent );
		void downloadSuccess( QVariantMap result );
		void serverChannelFailure();

		void changePushFailure( QString error, int flags );

		void serverSaveSuccess( QVariantMap results );
		void serverSaveFailure( QString error, int flags );

		void serverReconnectSuccess( QVariantMap results );
		void serverReconnectFailure( QString error, int flags );

	signals:
		void resyncSuccessRethreadSignal( int );

	protected:
		virtual ~ServerFile();

		void finalizeFileOpen();
		virtual void handleDocumentChange( int position, int removeChars, const QString &insert );
		virtual void setLastSavedRevision( int lastSavedRevision );
		void pumpChangeQueue();
		void movePumpCursor( int revision );
		void reconnect();

	private:
		SshHost *mHost;

		QList< Change * > mChangesSinceLastSave;
		int mChangePumpCursor;

// Temporary stuff used during opening
		inline void clearTempOpenData() {
			mServerOpenResults.clear(); mDownloadedData = QByteArray(); mDownloadedChecksum = QByteArray();
		}

		QVariantMap mServerOpenResults;
		QByteArray mDownloadedData;
		QByteArray mDownloadedChecksum;
};

#endif  // SSHFILE_H
