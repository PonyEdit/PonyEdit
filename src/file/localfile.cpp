#include <QDebug>

#include "localfile.h"

LocalFile::LocalFile( const Location &location ) :
	BaseFile( location ) {
	connect( this,
	         SIGNAL(localFileOpened(QString,QByteArray,bool)),
	         this,
	         SLOT(openSuccess(QString,QByteArray,bool)),
	         Qt::QueuedConnection );
}

BaseFile *LocalFile::newFile( const QString &content ) {
	mContent = content;

	save();

	emit localFileOpened( content, getChecksum().toLatin1(), false );

	return this;
}

void LocalFile::open() {
	QFile fileHandle( mLocation.getPath() );
	fileHandle.open( QIODevice::ReadOnly );

	bool readOnly = false;
	if ( ! ( fileHandle.permissions() & QFile::WriteUser ) ) {
		readOnly = true;
	}

	QTextStream stream( &fileHandle );

	QString content = stream.readAll();

	fileHandle.close();

	mContent = content;

	emit localFileOpened( content, getChecksum().toLatin1(), readOnly );
}

void LocalFile::save() {
	QFile fileHandle( mLocation.getPath() );
	fileHandle.open( QIODevice::WriteOnly );

	if ( ! ( fileHandle.permissions() & QFile::WriteUser ) ) {
		saveFailure( tr( "Permission denied!" ), true );
		return;
	}

	QTextStream stream( &fileHandle );

	stream << mContent;
	stream.flush();

	fileHandle.close();

	savedRevision( mRevision, mDocument->availableUndoSteps(), QByteArray( getChecksum().toLatin1() ) );
}

void LocalFile::close() {
	setOpenStatus( Closing );
	BaseFile::closeCompleted();
}

void LocalFile::refresh() {
	open();
}
