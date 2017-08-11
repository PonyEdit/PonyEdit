#include <QDebug>
#include <QDockWidget>
#include <QtAlgorithms>

#include "editorpanel.h"
#include "file/openfilemanager.h"
#include "globaldispatcher.h"
#include "windowmanager.h"

WindowManager* gWindowManager = NULL;

WindowManager::WindowManager( QWidget *parent ) :
	QWidget( parent ) {
	mEditorSelectionLocked = false;
	mParent = ( MainWindow * ) parent;
	mCurrentEditorPanel = NULL;
	gWindowManager = this;

	// Create a root editor stack
	mLayout = new QVBoxLayout( this );
	mLayout->setMargin( 0 );
	mRootEditorPanel = new EditorPanel( this );
	mLayout->addWidget( mRootEditorPanel );
	setCurrentEditorPanel( mRootEditorPanel );

	createSearchBar();
	createRegExpTester();
	createSearchResults();

	connect( &gOpenFileManager,
	         SIGNAL( fileClosed( BaseFile* ) ),
	         this,
	         SLOT( fileClosed( BaseFile* ) ),
	         Qt::DirectConnection );
}

WindowManager::~WindowManager() {
	mSearchBar->deleteLater();
	mSearchBarWrapper->deleteLater();
	mRegExpTester->deleteLater();
	mRegExpTesterWrapper->deleteLater();
}

void WindowManager::displayFile( BaseFile *file ) {
	if ( mCurrentEditorPanel != NULL ) {
		mCurrentEditorPanel->displayFile( file );

		Editor* e = currentEditor();
		if ( e != NULL ) {
			e->setFocus();
		}
	}
}

void WindowManager::fileClosed( BaseFile *file ) {
	// Tell all editor stacks that the file's gone.
	mRootEditorPanel->fileClosed( file );
}

void WindowManager::setCurrentEditorPanel( EditorPanel* stack ) {
	if ( mEditorSelectionLocked ) {
		return;
	}

	if ( mCurrentEditorPanel == stack ) {
		return;
	}

	if ( mCurrentEditorPanel != NULL ) {
		mCurrentEditorPanel->setActive( false );
	}

	mCurrentEditorPanel = stack;

	if ( mCurrentEditorPanel != NULL ) {
		mCurrentEditorPanel->setActive( true );
	}
}

BaseFile* WindowManager::getCurrentFile() {
	EditorPanel* currentPanel = getCurrentPanel();
	if ( ! currentPanel ) {
		return NULL;
	}

	Editor* currentEditor = currentPanel->getCurrentEditor();
	if ( ! currentEditor ) {
		return NULL;
	}

	return currentEditor->getFile();
}

void WindowManager::nextWindow() {
	BaseFile* file = gOpenFileManager.getNextFile( getCurrentFile() );
	if ( file ) {
		gDispatcher->emitSelectFile( file );
	}
}

void WindowManager::previousWindow() {
	BaseFile* file = gOpenFileManager.getPreviousFile( getCurrentFile() );
	if ( file ) {
		gDispatcher->emitSelectFile( file );
	}
}

void WindowManager::findInCurrentEditor( const QString &text, bool backwards, bool caseSensitive, bool useRegexp ) {
	Editor* current = currentEditor();
	if ( current ) {
		current->find( text, backwards, caseSensitive, useRegexp );
	}
}

int WindowManager::find( Editor *editor,
                         const QString &text,
                         bool backwards,
                         bool caseSensitive,
                         bool useRegexp,
                         bool loop ) {
	return editor->find( text, backwards, caseSensitive, useRegexp, loop );
}

void WindowManager::replaceInCurrentEditor( const QString &text, const QString &replaceText, bool all ) {
	Editor* current = currentEditor();
	if ( current ) {
		current->replace( text, replaceText, false, false, all );
	}
}

void WindowManager::findNext() {
	mSearchBar->findNext();
}

void WindowManager::findPrevious() {
	mSearchBar->findPrev();
}

void WindowManager::createSearchBar() {
	mSearchBar = new SearchBar();
	mSearchBarWrapper = new QDockWidget( "Search", 0, Qt::FramelessWindowHint );
	mSearchBarWrapper->setFeatures( QDockWidget::DockWidgetClosable );
	mSearchBarWrapper->setWidget( mSearchBar );

	mParent->addDockWidget( Qt::BottomDockWidgetArea, mSearchBarWrapper, Qt::Horizontal );
	mParent->registerContextMenuItem( mSearchBarWrapper );

	mSearchBarWrapper->hide();
	mSearchBarWrapper->setTitleBarWidget( new QWidget( this ) );
	connect( mSearchBar, SIGNAL( closeRequested() ), this, SLOT( hideSearchBar() ) );
	connect( mSearchBar, SIGNAL( find( QString, bool ) ), this, SLOT( findInCurrentEditor( QString, bool ) ) );
	connect( mSearchBar,
	         SIGNAL( replace( QString, QString, bool ) ),
	         this,
	         SLOT( replaceInCurrentEditor( QString, QString, bool ) ) );
	mSearchBarWrapper->setObjectName( "Search Bar" );
}

void WindowManager::createSearchResults() {
	mSearchResults = new SearchResults();
	mSearchResultsWrapper = new QDockWidget( "Search Results" );
	mSearchResultsWrapper->setWidget( mSearchResults );

	mParent->addDockWidget( Qt::BottomDockWidgetArea, mSearchResultsWrapper, Qt::Horizontal );

	mSearchResultsWrapper->hide();
	mSearchResultsWrapper->setObjectName( "Search Results" );
}

void WindowManager::showSearchBar() {
	mSearchBarWrapper->show();
	mSearchBar->takeFocus();
}

void WindowManager::hideSearchBar() {
	mSearchBarWrapper->hide();
	Editor* editor = currentEditor();
	if ( editor ) {
		editor->getCodeEditor()->setFocus();
	}
}

void WindowManager::createRegExpTester() {
	mRegExpTester = new RegExpTester();
	mRegExpTesterWrapper = new QDockWidget( tr( "Regular Expression Tester" ), 0 );
	mRegExpTesterWrapper->setWidget( mRegExpTester );

	mParent->addDockWidget( Qt::BottomDockWidgetArea, mRegExpTesterWrapper, Qt::Horizontal );
	mParent->registerContextMenuItem( mRegExpTesterWrapper );

	mRegExpTesterWrapper->hide();
	mRegExpTesterWrapper->setObjectName( "Search Bar" );
}

void WindowManager::showRegExpTester() {
	mRegExpTesterWrapper->show();

	Editor* editor = currentEditor();
	QString selectedText;
	if ( editor ) {
		CodeEditor* codeEditor = editor->getCodeEditor();
		selectedText = codeEditor->textCursor().selectedText();
	}

	mRegExpTester->takeFocus( selectedText );
}

void WindowManager::notifyEditorChanged( EditorPanel* stack ) {
	if ( stack == mCurrentEditorPanel ) {
		emit currentChanged();
	}
}

void WindowManager::splitVertically() {
	EditorPanel* panel = mCurrentEditorPanel;
	if ( ! panel ) {
		return;
	}

	panel->split( Qt::Horizontal );
	setCurrentEditorPanel( panel->getFirstChild() );
	emit splitChanged();
}

void WindowManager::splitHorizontally() {
	EditorPanel* panel = mCurrentEditorPanel;
	if ( ! panel ) {
		return;
	}

	mCurrentEditorPanel->split( Qt::Vertical );
	setCurrentEditorPanel( panel->getFirstChild() );
	emit splitChanged();
}

Editor* WindowManager::currentEditor() {
	if ( mCurrentEditorPanel == NULL ) {
		return NULL;
	}

	return mCurrentEditorPanel->getCurrentEditor();
}

bool WindowManager::isSplit() {
	return mRootEditorPanel->isSplit();
}

void WindowManager::removeSplit() {
	if ( mCurrentEditorPanel == NULL ) {
		return;
	}

	mCurrentEditorPanel->unsplit();
	emit splitChanged();
}

void WindowManager::removeAllSplits() {
	mRootEditorPanel->unsplit();
	emit splitChanged();
}

void WindowManager::nextSplit() {
	if ( mCurrentEditorPanel == NULL ) {
		return;
	}
	EditorPanel* nextPanel = mCurrentEditorPanel->findNextPanel();
	if ( nextPanel ) {
		setCurrentEditorPanel( nextPanel );
	}
}

void WindowManager::previousSplit() {
	if ( mCurrentEditorPanel == NULL ) {
		return;
	}
	EditorPanel* nextPanel = mCurrentEditorPanel->findPreviousPanel();
	if ( nextPanel ) {
		setCurrentEditorPanel( nextPanel );
	}
}

EditorPanel* WindowManager::getFirstPanel() {
	EditorPanel* panel = mRootEditorPanel;
	while ( panel->isSplit() ) {
		panel = panel->getFirstChild();
	}
	return panel;
}

EditorPanel* WindowManager::getLastPanel() {
	EditorPanel* panel = mRootEditorPanel;
	while ( panel->isSplit() ) {
		panel = panel->getSecondChild();
	}
	return panel;
}

void WindowManager::searchInFiles( const QList< BaseFile* > files,
                                   const QString& text,
                                   bool caseSensitive,
                                   bool useRegExp,
                                   bool showReplaceOptions ) {
	QList< SearchResultModel::Result > results;

	foreach ( BaseFile * file, files ) {
		QTextDocument* doc = file->getTextDocument();
		QTextCursor cursor( doc );
		while ( ! ( cursor =
				    Editor::find( doc,
				                  cursor,
				                  text,
				                  false,
				                  caseSensitive,
				                  useRegExp,
				                  false ) ).isNull() ) {
			results.append( SearchResultModel::Result( cursor.block().text(),
			                                           file->getLocation(),
			                                           cursor.blockNumber(),
			                                           cursor.selectionStart() - cursor.block().position(),
			                                           cursor.selectionEnd() - cursor.selectionStart() ) );
		}
	}

	showSearchResults( results, showReplaceOptions );
}

void WindowManager::showSearchResults( const QList< SearchResultModel::Result >& results, bool showReplaceOptions ) {
	mSearchResults->showResults( results );
	mSearchResults->showReplaceOptions( showReplaceOptions );
	mSearchResultsWrapper->show();
}

void WindowManager::showAndSelect( const Location& location, int lineNumber, int start, int length ) {
	displayLocation( location );
	Editor* editor = currentEditor();
	if ( editor != NULL ) {
		editor->selectText( lineNumber, start, length );
	}
}

void WindowManager::displayLocation( const Location& location ) {
	BaseFile* file = gOpenFileManager.getFile( location );
	if ( file != NULL ) {
		displayFile( file );
	}
}

void WindowManager::hideSearchResults() {
	mSearchResultsWrapper->hide();
}
