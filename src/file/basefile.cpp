#include <QCryptographicHash>
#include <QDebug>

#include "basefile.h"
#include "editor/editor.h"
#include "file/openfilemanager.h"
#include "ftpfile.h"
#include "localfile.h"
#include "main/dialogwrapper.h"
#include "main/globaldispatcher.h"
#include "main/statuswidget.h"
#include "main/tools.h"
#include "QsLog.h"
#include "serverfile.h"
#include "syntax/syntaxdefinition.h"
#include "syntax/syntaxdefmanager.h"
#include "syntax/syntaxhighlighter.h"
#include "unsavedfile.h"

const char *BaseFile::sStatusLabels[] = {
	"Loading...",
	"Error while loading",
	"Ready",
	"Disconnected",
	"Reconnecting...",
	"Lost Synchronization; Repairing",
	"Syncronization Error",
	"Closing",
	"Closed"
};
QMutex BaseFile::sDeletionLock;

BaseFile *BaseFile::getFile( const Location &location ) {
	// See if the specified location is already open...
	BaseFile *existingFile = gOpenFileManager.getFile( location );
	if ( existingFile ) {
		return existingFile;
	}

	// If not, create a new file object.
	BaseFile *newFile = nullptr;
	Location::Protocol protocol = location.getProtocol();
	switch ( protocol ) {
		case Location::Ssh:
			newFile = new ServerFile( location );
			break;

		case Location::Local:
			newFile = new LocalFile( location );
			break;

		case Location::Sftp:
			newFile = new FtpFile( location );
			break;

		case Location::Unsaved:
			newFile = new UnsavedFile( location );
	}

	if ( newFile ) {
		gOpenFileManager.registerFile( newFile );
	}

	return newFile;
}

BaseFile::~BaseFile() {
	// Tell every attached editor
	foreach ( Editor *editor, mAttachedEditors ) {
		editor->fileClosed();
	}

	delete mDocument;
}

BaseFile::BaseFile( const Location &location ) :
	mLocation( location ),
	mContent( "" ),
	mError( "" ),
	mDocument( new QTextDocument( this ) ),
	mDocumentLayout( new QPlainTextDocumentLayout( mDocument ) ),
	mChanged( false ),
	mDosLineEndings( false ),
	mReadOnly( false ),
	mIgnoreChanges( 0 ),
	mInUndoBlock( 0 ),
	mInRedoBlock( 0 ),
	mRevision( 0 ),
	mLastSavedRevision( 0 ),
	mLastSavedUndoLength( 0 ),
	mLastSaveChecksum( nullptr ),
	mProgress( -1 ),
	mOpenStatus( BaseFile::Closed ),
	mHighlighter( nullptr ) {
	mDocument->setDocumentLayout( mDocumentLayout );

	connect( mDocument, SIGNAL( contentsChange( int, int, int ) ), this, SLOT( documentChanged( int, int, int ) ) );
	connect( this,
	         SIGNAL( fileOpenedRethreadSignal( QString, QByteArray, bool ) ),
	         this,
	         SLOT( openSuccess( QString, QByteArray, bool ) ),
	         Qt::QueuedConnection );
	connect( this, SIGNAL( closeCompletedRethreadSignal() ), this, SLOT( closeCompleted() ), Qt::QueuedConnection );
	connect( this,
	         SIGNAL( saveFailedRethreadSignal( QString, bool ) ),
	         this,
	         SLOT( saveFailure( QString, bool ) ),
	         Qt::QueuedConnection );
}

void BaseFile::documentChanged( int position, int removeChars, int charsAdded ) {
	QString added = "";
	for ( int i = 0; i < charsAdded; i++ ) {
		QChar c = mDocument->characterAt( i + position );
		if ( c == QChar::ParagraphSeparator || c == QChar::LineSeparator ) {
			c = QLatin1Char( '\n' );
		} else if ( c == QChar::Nbsp ) {
			c = QLatin1Char( ' ' );
		}

		added += c;
	}

	this->handleDocumentChange( position, removeChars, added );
}

void BaseFile::handleDocumentChange( int position, int removeChars, const QString &insert ) {
	if ( mIgnoreChanges ) {
		return;
	}

	mContent.replace( position, removeChars, insert );
	mRevision++;

	int undoLength = mDocument->availableUndoSteps();
	if ( undoLength <= mLastSavedUndoLength && ! mInRedoBlock && ! mInUndoBlock ) {
		// If making changes when undo'd past last save, you can never reach "no unsaved changes" w/o saving.
		mLastSavedUndoLength = -1;
	}

	// Make doc as unchanged if undone to last save, otherwise mark as changed.
	bool newChangedState = ( undoLength != mLastSavedUndoLength );
	if ( newChangedState != mChanged ) {
		mChanged = newChangedState;
		emit unsavedStatusChanged();
	}
}

void BaseFile::openSuccess( const QString &content, const QByteArray &checksum, bool readOnly ) {
	// If this is not the main thread, move to the main thread.
	if ( ! Tools::isMainThread() ) {
		emit fileOpenedRethreadSignal( content, checksum, readOnly );
		return;
	}

	mLastSaveChecksum = checksum;
	mContent = content;
	mReadOnly = readOnly;

	// Detect line ending mode, then convert it to unix-style. Use unix-style line endings everywhere, only convert
	// to DOS at save time.
	mDosLineEndings = mContent.contains( "\r\n" );
	if ( mDosLineEndings ) {
		mContent.replace( "\r\n", "\n" );
	}

	ignoreChanges();
	autodetectSyntax();
	mDocument->setPlainText( content );
	unignoreChanges();

	mDocument->clearUndoRedoStacks();
	savedRevision( mRevision, mDocument->availableUndoSteps(), QByteArray( getChecksum().toLatin1() ) );

	setOpenStatus( Ready );
}

void BaseFile::openFailure( const QString &error, int /*errorFlags*/ ) {
	// TODO: Check the errorFlags for a permission error, to offer a SUDO option.
	mError = error;
	setOpenStatus( LoadError );
}

void BaseFile::setOpenStatus( OpenStatus newStatus ) {
	mOpenStatus = newStatus;
	mProgress = -1;
	emit openStatusChanged( newStatus );
}

void BaseFile::editorAttached( Editor *editor ) {
	// Call only from Editor constructor.
	mAttachedEditors.append( editor );
}

void BaseFile::editorDetached( Editor *editor ) {
	// Call only from Editor destructor.
	mAttachedEditors.removeOne( editor );
}

QString BaseFile::getChecksum( const QByteArray &content ) {
	QCryptographicHash hash( QCryptographicHash::Md5 );
	hash.addData( content );
	return QString( hash.result().toHex().toLower() );
}

QString BaseFile::getChecksum() const {
	return getChecksum( mContent.toUtf8() );
}

void BaseFile::savedRevision( int revision, int undoLength, const QByteArray &checksum ) {
	mLastSaveChecksum = checksum;
	setLastSavedRevision( revision );

	mLastSavedUndoLength = undoLength;
	mChanged = ( undoLength != mDocument->availableUndoSteps() );

	gDispatcher->emitGeneralStatusMessage( QString( "Finished saving " ) + mLocation.getLabel() );
	QLOG_TRACE() << "Saved file" << mLocation.getLabel() << "at revision" << revision;
	emit unsavedStatusChanged();
}

void BaseFile::setLastSavedRevision( int lastSavedRevision ) {
	mLastSavedRevision = lastSavedRevision;
}

void BaseFile::setProgress( int percent ) {
	mProgress = percent;
	emit fileProgress( percent );
}

const Location &BaseFile::getDirectory() const {
	return mLocation.getDirectory();
}

void BaseFile::closeCompleted() {
	// If this is not the main thread, move to the main thread.
	if ( ! Tools::isMainThread() ) {
		emit closeCompletedRethreadSignal();
		return;
	}

	sDeletionLock.lock();

	setOpenStatus( Closed );
	gOpenFileManager.deregisterFile( this );
	delete this;

	sDeletionLock.unlock();
}

void BaseFile::autodetectSyntax() {
	setSyntax( gSyntaxDefManager->getDefinitionForFile( getLocation().getPath() ) );
}

QString BaseFile::getSyntax() const {
	if ( ! mHighlighter ) {
		return QString();
	}

	return mHighlighter->getSyntaxDefinition()->getSyntaxName();
}

void BaseFile::setSyntax( const QString &syntaxName ) {
	setSyntax( gSyntaxDefManager->getDefinitionForSyntax( syntaxName ) );
}

void BaseFile::setSyntax( SyntaxDefinition *syntaxDef ) {
	ignoreChanges();
	if ( syntaxDef ) {
		if ( ! mHighlighter ) {
			mHighlighter = new SyntaxHighlighter( mDocument, syntaxDef );
			mHighlighter->rehighlight();
		} else {
			mHighlighter->setSyntaxDefinition( syntaxDef );
		}
	} else if ( mHighlighter ) {
		delete mHighlighter;
		mHighlighter = nullptr;
	}
	unignoreChanges();

	gDispatcher->emitSyntaxChanged( this );
}

void BaseFile::saveFailure( const QString &errorMessage, bool permissionError ) {
	// Make sure this is always run in the UI thread
	if ( ! Tools::isMainThread() ) {
		emit saveFailedRethreadSignal( errorMessage, permissionError );
		return;
	}

	auto *errorWidget = new StatusWidget( true );
	errorWidget->setStatus( QPixmap( ":/icons/error.png" ), errorMessage );
	errorWidget->setButtons( StatusWidget::Retry | StatusWidget::Cancel |
	                         ( permissionError && mLocation.canSudo() &&
	                           ! mLocation.isSudo() ? StatusWidget::SudoRetry : StatusWidget::None ) );
	errorWidget->setCloseOnButton( true );

	DialogWrapper< StatusWidget > errorDialog( tr( "Error Saving File" ), errorWidget, false );
	errorDialog.exec();

	StatusWidget::Button button = errorWidget->getResult();
	switch ( button ) {
		case StatusWidget::Retry:
			save();
			break;

		case StatusWidget::SudoRetry:
			sudo();
			save();
			break;

		case StatusWidget::None:
		case StatusWidget::Cancel:
		case StatusWidget::ShowLog:
		case StatusWidget::Done:
		case StatusWidget::Connect:
			break;
	}
}

void BaseFile::beginRedoBlock() {
	mInRedoBlock++;
}

void BaseFile::beginUndoBlock() {
	mInUndoBlock++;
}

void BaseFile::endUndoBlock() {
	mInUndoBlock--;
	if ( mInUndoBlock < 0 ) {
		mInUndoBlock = 0;
	}
}

void BaseFile::endRedoBlock() {
	mInRedoBlock--;
	if ( mInRedoBlock < 0 ) {
		mInRedoBlock = 0;
	}
}

void BaseFile::sudo() {
	throw( tr( "Invalid operation: Sudo called on incompatible file type" ) );
}
