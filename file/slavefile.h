#ifndef SSHFILE_H
#define SSHFILE_H

#include "basefile.h"

class OldSlaveChannel;
class RemoteConnection;
class SlaveFile : public BaseFile
{
Q_OBJECT

public:
BaseFile* newFile( const QString& content );
void open();
void save();
void close();
void refresh();

SlaveFile( const Location& location );		// Do not call; use File::getFile instead

virtual void sudo();

public slots:
void createSuccess( QVariantMap result );

void slaveOpenSuccess( QVariantMap results );
void downloadProgress( int percent );
void downloadSuccess( QVariantMap result );
void slaveChannelFailure();

void changePushFailure( QString error, int flags );

void slaveSaveSuccess( QVariantMap results );
void slaveSaveFailure( QString error, int flags );

void slaveReconnectSuccess( QVariantMap results );
void slaveReconnectFailure( QString error, int flags );

signals:
void resyncSuccessRethreadSignal( int );

protected:
virtual ~SlaveFile();

void finalizeFileOpen();
virtual void handleDocumentChange( int position, int removeChars, const QString& insert );
virtual void setLastSavedRevision( int lastSavedRevision );
void pumpChangeQueue();
void movePumpCursor( int revision );
void reconnect();

private:
SshHost* mHost;

QList< Change* > mChangesSinceLastSave;
int mChangePumpCursor;

// Temporary stuff used during opening
inline void clearTempOpenData() {
	mSlaveOpenResults.clear(); mDownloadedData = QByteArray(); mDownloadedChecksum = QByteArray();
}

QVariantMap mSlaveOpenResults;
QByteArray mDownloadedData;
QByteArray mDownloadedChecksum;
};

#endif	// SSHFILE_H
