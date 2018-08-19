#include <QCryptographicHash>
#include "ftpfile.h"
#include "ssh2/sftprequest.h"
#include "ssh2/sshhost.h"
#include "tools/callback.h"

FtpFile::FtpFile( const Location &location ) :
	BaseFile( location ) {
	mHost = location.getRemoteHost();
}

BaseFile *FtpFile::newFile( const QString &content ) {
	openSuccess( content, getChecksum( content.toUtf8() ).toLatin1(), false );
	save();
	return this;
}

void FtpFile::open() {
	setOpenStatus( BaseFile::Loading );

	SFTPRequest *request =
		new SFTPRequest( SFTPRequest::ReadFile,
		                 Callback( this,
		                           SLOT(sftpReadSuccess(QVariantMap)),
		                           SLOT(sftpReadFailure(QString,int)),
		                           SLOT(sftpReadProgress(int)) ) );
	request->setPath( mLocation.getRemotePath() );
	mHost->sendSftpRequest( request );
}

void FtpFile::sftpReadSuccess( const QVariantMap &results ) {
	QByteArray content = results.value( "content", QByteArray() ).toByteArray();
	QByteArray checksum = BaseFile::getChecksum( content ).toLatin1();

	openSuccess( QString::fromUtf8( content, content.size() ), checksum, false );
}

void FtpFile::sftpReadFailure( const QString &error, int flags ) {
	openFailure( error, flags );
}

void FtpFile::sftpReadProgress( int progress ) {
	setProgress( progress );
}

void FtpFile::save() {
	setProgress( 0 );

	SFTPRequest *request =
		new SFTPRequest( SFTPRequest::WriteFile,
		                 Callback( this,
		                           SLOT(sftpWriteSuccess(QVariantMap)),
		                           SLOT(sftpWriteFailure(QString,int)),
		                           SLOT(sftpWriteProgress(int)) ) );
	request->setPath( mLocation.getRemotePath() );
	request->setContent( mContent.toUtf8() );
	request->setRevision( mRevision );
	request->setUndoLength( mDocument->availableUndoSteps() );
	mHost->sendSftpRequest( request );
}

void FtpFile::sftpWriteSuccess( const QVariantMap &results ) {
	setProgress( -1 );

	int revision = results.value( "revision" ).toInt();
	int undoLength = results.value( "undoLength" ).toInt();
	QByteArray checksum = results.value( "checksum" ).toByteArray();

	savedRevision( revision, undoLength, checksum );
}

void FtpFile::sftpWriteFailure( const QString &error, int /*lags*/ ) {
	setProgress( -1 );
	saveFailure( error, false );
}

void FtpFile::sftpWriteProgress( int progress ) {
	setProgress( progress );
}

bool FtpFile::canClose() {
	return true;
}

void FtpFile::close() {
	setOpenStatus( Closing );
	BaseFile::closeCompleted();
}

void FtpFile::refresh() {
	open();
}
