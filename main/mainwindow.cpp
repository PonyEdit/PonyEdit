#include <QApplication>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDesktopServices>
#include <QErrorMessage>
#include <QHash>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRegExp>
#include <QSettings>
#include <QSyntaxHighlighter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QTextEdit>
#include <QTime>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

#include "aboutdialog.h"
#include "advancedsearchdialog.h"
#include "editor/editor.h"
#include "file/filedialog.h"
#include "file/filelist.h"
#include "file/openfilemanager.h"
#include "file/openfilemanager.h"
#include "file/tabbedfilelist.h"
#include "file/unsavedchangesdialog.h"
#include "gotolinedialog.h"
#include "main/globaldispatcher.h"
#include "main/mainwindow.h"
#include "options/options.h"
#include "options/optionsdialog.h"
#include "QsLog.h"
#include "shutdownprompt.h"
#include "syntax/syntaxdefmanager.h"
#include "tools.h"
#include "tools/htmlpreview.h"
#include "website/sitemanager.h"


MainWindow::MainWindow( QWidget *parent )
	: QMainWindow( parent ) {
	mUnsavedChangesDialog = NULL;
	mCurrentSyntaxMenuItem = NULL;

	setWindowTitle( tr( "PonyEdit" ) );

	setAcceptDrops( true );

	gWindowManager = new WindowManager( this );
	gWindowManager->setMinimumWidth( 200 );
	setCentralWidget( gWindowManager );

	mFileList = new FileList();
	addDockWidget( Qt::LeftDockWidgetArea, mFileList, Qt::Vertical );
	mFileList->setObjectName( "File List" );

	mTabbedFileList = new TabbedFileList();
	addDockWidget( Qt::TopDockWidgetArea, mTabbedFileList, Qt::Horizontal );
	mTabbedFileList->setObjectName( "Tabbed File List" );

	if ( Options::FileListType == Options::QuickList ) {
		mFileList->setVisible( true );
		mTabbedFileList->setVisible( false );
	} else {
		mFileList->setVisible( false );
		mTabbedFileList->setVisible( true );
	}

	mStatusLine = new QLabel();
	mStatusBar = new QStatusBar();
	mStatusBar->addPermanentWidget( mStatusLine );
	setStatusBar( mStatusBar );

	createToolbar();

	createFileMenu();
	createEditMenu();
	createViewMenu();
	createToolsMenu();
	createWindowMenu();
	createHelpMenu();
	createMacDockMenu();

	createShortcuts();

	connect( gDispatcher,
	         SIGNAL( generalErrorMessage( QString ) ),
	         this,
	         SLOT( showErrorMessage( QString ) ),
	         Qt::QueuedConnection );
	connect( gDispatcher,
	         SIGNAL( generalStatusMessage( QString ) ),
	         this,
	         SLOT( showStatusMessage( QString ) ),
	         Qt::QueuedConnection );
	connect( gDispatcher, SIGNAL( selectFile( BaseFile* ) ), this, SLOT( fileSelected( BaseFile* ) ) );
	connect( gWindowManager,
	         SIGNAL( currentChanged() ),
	         this,
	         SLOT( currentEditorChanged() ),
	         Qt::QueuedConnection );
	connect( gDispatcher, SIGNAL( syntaxChanged( BaseFile* ) ), this, SLOT( updateSyntaxSelection() ) );
	connect( &gOpenFileManager, SIGNAL( fileOpened( BaseFile* ) ), this, SLOT( openFileListChanged() ) );
	connect( &gOpenFileManager, SIGNAL( fileClosed( BaseFile* ) ), this, SLOT( openFileListChanged() ) );
	connect( gWindowManager, SIGNAL( splitChanged() ), this, SLOT( viewSplittingChanged() ) );

	mRecentFiles = Tools::loadRecentFiles();
	updateRecentFilesMenu();

	// Set the default size to something relatively sane
	resize( QSize( 800, 600 ) );
	restoreState();

	openFileListChanged();
	viewSplittingChanged();
}

MainWindow::~MainWindow() {
	delete gWindowManager;
}

void MainWindow::restoreState() {
	QSettings settings;
	restoreGeometry( settings.value( "mainwindow/geometry" ).toByteArray() );
	QMainWindow::restoreState( settings.value( "mainwindow/state" ).toByteArray() );
	gWindowManager->hideSearchResults();
}

void MainWindow::createToolbar() {
	//
	// Main toolbar
	//

	QToolBar* toolbar = new QToolBar( "File Toolbar" );
	toolbar->addAction( QIcon( ":/icons/new.png" ), "New", this, SLOT( newFile() ) );
	toolbar->addAction( QIcon( ":/icons/open.png" ), "Open", this, SLOT( openFile() ) );
	mActionsRequiringFiles.append( toolbar->addAction( QIcon( ":/icons/save.png" ),
	                                                   "Save",
	                                                   this,
	                                                   SLOT(
								   saveFile() ) ) );
	toolbar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

	addToolBar( toolbar );
	registerContextMenuItem( toolbar );
	toolbar->setObjectName( "File Toolbar" );

	QWidget* spacer = new QWidget();
	spacer->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Expanding );
	toolbar->addWidget( spacer );

	toolbar->setStyleSheet( "QToolBar { margin: 0px; padding: 0px; spacing: 3px; }" );
	toolbar->setStyleSheet( "QToolButton { margin: 0px; padding: 0px; width:22px; height:22px }" );

	//
	// View toolbar
	//

	toolbar = new QToolBar( "View Toolbar" );
	mActionsRequiringFiles.append( toolbar->addAction( QIcon( ":/icons/verticalsplit.png" ),
	                                                   "Split View Vertically",
	                                                   gWindowManager,
	                                                   SLOT( splitVertically() ) ) );
	mActionsRequiringFiles.append( toolbar->addAction( QIcon( ":/icons/horizontalsplit.png" ),
	                                                   "Split View Horizontally",
	                                                   gWindowManager,
	                                                   SLOT( splitHorizontally() ) ) );
	mActionsRequiringSplitViews.append( toolbar->addAction( QIcon( ":/icons/removesplit.png" ),
	                                                        "Remove Split",
	                                                        gWindowManager,
	                                                        SLOT( removeSplit() ) ) );

	addToolBar( toolbar );
	registerContextMenuItem( toolbar );
	toolbar->setObjectName( "View Toolbar" );

	toolbar->setStyleSheet( "QToolBar { margin: 0px; padding: 0px; spacing: 3px; }" );
	toolbar->setStyleSheet( "QToolButton { margin: 0px; padding: 0px; width:22px; height:22px }" );
}

void MainWindow::newFile() {
	QString path = "";
	Location location( path );
	BaseFile* file = location.getFile();

	gWindowManager->displayFile( file );

	gDispatcher->emitSelectFile( file );

	connect( file, SIGNAL( openStatusChanged( int ) ), this, SLOT( updateTitle() ) );
	connect( file, SIGNAL( unsavedStatusChanged() ), this, SLOT( updateTitle() ) );
}

void MainWindow::openFile() {
	FileDialog dlg( this );
	if ( dlg.exec() ) {
		QList< Location > locations = dlg.getSelectedLocations();
		if ( locations.length() > 20 ) {
			QMessageBox msgBox;
			msgBox.setWindowTitle( tr( "Opening Many Files" ) );
			msgBox.setText( tr( "You have selected %1 files to open." ).arg( locations.length() ) );
			msgBox.setInformativeText( tr(
							   "This may take some time to complete. Are you sure you want to do this?" ) );
			msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
			msgBox.setDefaultButton( QMessageBox::Yes );

			if ( msgBox.exec() == QMessageBox::No ) {
				return;
			}
		}
		foreach ( Location location, locations ) {
			openSingleFile( location );
		}
	}
}

void MainWindow::openSingleFile() {
	QAction* action = static_cast< QAction* >( sender() );
	if ( action ) {
		openSingleFile( mRecentFiles[action->data().toInt()] );
	}
}

void MainWindow::openSingleFile( const Location& loc ) {
	if ( ! loc.isDirectory() ) {
		BaseFile* file;
		try{
			file = Location( loc ).getFile();
		}catch ( QString &e ) {
			QLOG_ERROR() << "Error opening a single file" << loc.getPath() << ": " << e;
			return;
		}

		if ( file->isClosed() ) {
			file->open();
		}

		gWindowManager->displayFile( file );

		gDispatcher->emitSelectFile( file );

		addRecentFile( loc );

		connect( file, SIGNAL( openStatusChanged( int ) ), this, SLOT( updateTitle() ) );
		connect( file, SIGNAL( unsavedStatusChanged() ), this, SLOT( updateTitle() ) );
	}
}

void MainWindow::saveFile() {
	Editor* current = gWindowManager->currentEditor();
	if ( current ) {
		if ( current->getFile()->getLocation().getProtocol() == Location::Unsaved ) {
			saveFileAs();
		} else {
			try{
				current->save();
			}catch ( QString &e ) {
				QLOG_ERROR() << "Error saving the curent file: " << e;
			}
		}
	}
}

void MainWindow::saveFileAs() {
	Editor* current = gWindowManager->currentEditor();
	if ( current == NULL ) {
		return;
	}

	FileDialog dlg( this, true );
	if ( dlg.exec() ) {
		Location loc = dlg.getNewLocation();
		try{
			if ( ! loc.isNull() ) {
				loc.getFile()->newFile( current->getFile()->getContent() );
			}
		}catch ( QString &e ) {
			QLOG_ERROR() << "Error saving file as: " << e;
			return;
		}

		addRecentFile( loc );

		if ( current->getFile()->getLocation().getProtocol() == Location::Unsaved ) {
			current->close();
		}
	}
}

void MainWindow::saveAllFiles() {
	QList< BaseFile* > unsavedFiles = gOpenFileManager.getUnsavedFiles( gOpenFileManager.getOpenFiles() );

	foreach ( BaseFile * file, unsavedFiles ) {
		file->save();
	}
}

void MainWindow::closeFile() {
	Editor* current = gWindowManager->currentEditor();
	if ( current ) {
		QList< BaseFile * > files;
		files.append( current->getFile() );

		gOpenFileManager.closeFiles( files );
	}
}

void MainWindow::closeAllFiles() {
	gOpenFileManager.closeAllFiles();
}

void MainWindow::closeAllExceptCurrentFile() {
	QList< BaseFile* > openFiles = gOpenFileManager.getOpenFiles();

	BaseFile* current = gWindowManager->currentEditor()->getFile();

	foreach ( BaseFile * file, openFiles ) {
		if ( file != current ) {
			file->close();
		}
	}
}

void MainWindow::fileSelected( BaseFile* file ) {
	if ( ! file ) {
		return;
	}

	updateTitle( file );

	gWindowManager->displayFile( file );
}

void MainWindow::updateTitle() {
	BaseFile* file = static_cast< BaseFile* >( sender() );
	if ( file->isClosed() ) {
		setWindowTitle( "PonyEdit" );
		return;
	}
	QList< Editor* > editors = file->getAttachedEditors();

	bool current = false;
	foreach ( Editor * editor, editors ) {
		if ( editor == gWindowManager->currentEditor() ) {
			current = true;
		}
	}

	if ( current ) {
		updateTitle( file );
	}
}

void MainWindow::updateTitle( BaseFile* file ) {
	bool modified = false;
	QString title = "PonyEdit - ";
	title += file->getLocation().getLabel();
	if ( file->hasUnsavedChanges() ) {
		modified = true;
	}

#ifdef Q_OS_MAC
	if ( file->getLocation().getProtocol() == Location::Local ) {
		setWindowFilePath( file->getLocation().getPath() );
	} else {
		setWindowFilePath( "" );
		QIcon icon = file->getLocation().getIcon();
		setWindowIcon( icon );
	}
#endif

	setWindowTitle( title );
	setWindowModified( modified );
}

void MainWindow::options() {
	OptionsDialog dlg( this );
	dlg.exec();
}

void MainWindow::about() {
	AboutDialog dlg( this );
	dlg.exec();
}

void MainWindow::contextHelp() {
	Editor* editor = this->getCurrentEditor();
	if ( ! editor ) {
		return;
	}

	QString text = editor->getCodeEditor()->textCursor().selectedText();
	QString ext = editor->getFile()->getLocation().getPath().split( '.' ).last();

	bool dashInstalled = false;

#ifdef Q_OS_MAC
	dashInstalled = QDir( "/Applications/Dash.app" ).exists();
#endif

	if ( dashInstalled ) {
		// Source for these hashes: https://kapeli.com/dash_plugins

		QHash< QString, QString > keys;

		// Language group name -> set of documentations for Dash to use
		keys["actionscript"] = "actionscript";
		keys["android"] = "android";
		keys["boo"] = "unity3d";
		keys["c"] = "c,glib,gl2,gl3,gl4,manpages";
		keys["c++"] = "cpp,net,boost,qt,cvcpp,cocos2dx,c,manpages";
		keys["c#"] = "net,mono,unity3d";
		keys["cappuccino"] = "cappuccino";
		keys["clojure"] = "clojure";
		keys["coffeescript"] = "coffeescript";
		keys["coldfusion"] = "coldfusion";
		keys["config-files"] = "nginx";
		keys["css"] = "css,bootstrap,foundation,less,awesome,cordova,phonegap";
		keys["dart"] = "dartlang,polymerdart,angulardart";
		keys["elixir"] = "elixir";
		keys["erlang"] = "erlang";
		keys["go"] = "go,godoc";
		keys["gradle"] = "gradle";
		keys["haskell"] = "haskell";
		keys["haml"] = "haml";
		keys["html"] =
			"html,svg,css,bootstrap,foundation,awesome,statamic,javascript,jquery,jqueryui,jquerym,angularjs,backbone,marionette,meteor,moo,prototype,ember,lodash,underscore,sencha,extjs,knockout,zepto,cordova,phonegap,yui";
		keys["jade"] = "jade";
		keys["java"] = "java,javafx,grails,groovy,playjava,spring,cvj,processing";
		keys["javascript"] =
			"javascript,jquery,jqueryui,jquerym,angularjs,backbone,marionette,meteor,sproutcore,moo,prototype,bootstrap,foundation,lodash,underscore,ember,sencha,extjs,titanium,knockout,zepto,yui,d3,svg,dojo,coffee,nodejs,express,grunt,mongoose,moment,require,awsjs,jasmine,sails,sinon,chai,html,css,cordova,phonegap,unity3d";
		keys["julia"] = "julia";
		keys["less"] = "less";
		keys["lisp"] = "lisp";
		keys["lua"] = "lua,corona";
		keys["markdown"] = "markdown";
		keys["objective-c"] =
			"iphoneos,macosx,watchos,tvos,appledoc,cocos2d,cocos3d,kobold2d,sparrow,c,manpages";
		keys["objective-c++"] =
			"cpp,iphoneos,macosx,appledoc,cocos2dx,cocos2d,cocos3d,kobold2d,sparrow,c,manpages";
		keys["ocaml"] = "ocaml";
		keys["perl"] = "perl,manpages";
		keys["php"] =
			"php,wordpress,drupal,zend,laravel,yii,joomla,ee,codeigniter,cakephp,phpunit,symfony,typo3,twig,smarty,craft,phpp,html,statamic,mysql,sqlite,mongodb,psql,redis";
		keys["processing"] = "processing";
		keys["pug"] = "pug";
		keys["puppet"] = "puppet";
		keys["python"] =
			"python,django,twisted,sphinx,flask,tornado,sqlalchemy,numpy,scipy,salt,pandas,matplotlib,cvp";
		keys["r"] = "r";
		keys["racket"] = "racket";
		keys["ruby"] = "ruby,rubygems,rails";
		keys["rust"] = "rust";
		keys["sass"] = "sass,compass,bourbon,neat,susy,css";
		keys["scala"] = "scala,akka,playscala";
		keys["shell-scripts"] = "bash,manpages";
		keys["sql"] = "mysql,sqlite,psql";
		keys["stylus"] = "stylus";
		keys["swift"] = "swift,iphoneos,macosx,watchos,tvos,appledoc";
		keys["tcl"] = "tcl";
		keys["typescript"] = "typescript";
		keys["yaml"] = "chef,ansible";

		QHash< QString, QString > extensions;

		// File extension -> language group name
		extensions["as"] = "actionscript";
		extensions["as3"] = "actionscript";
		extensions["boo"] = "boo";
		extensions["c"] = "c";
		extensions["h"] = "c";
		extensions["cpp"] = "c++";
		extensions["cc"] = "c++";
		extensions["cp"] = "c++";
		extensions["cxx"] = "c++";
		extensions["c++"] = "c++";
		extensions["C"] = "c++";
		extensions["hh"] = "c++";
		extensions["hpp"] = "c++";
		extensions["hxx"] = "c++";
		extensions["h++"] = "c++";
		extensions["ini"] = "c++";
		extensions["ipp"] = "c++";
		extensions["cs"] = "c#";
		extensions["j"] = "cappuccino";
		extensions["clj"] = "clojure";
		extensions["edn"] = "clojure";
		extensions["coffee"] = "coffeescript";
		extensions["cfm"] = "coldfusion";
		extensions["cfml"] = "coldfusion";
		extensions["config"] = "config-files";
		extensions["css"] = "css";
		extensions["dart"] = "dart";
		extensions["ex"] = "elixir";
		extensions["exs"] = "elixir";
		extensions["erl"] = "erlang";
		extensions["hrl"] = "erlang";
		extensions["go"] = "go";
		extensions["gradle"] = "gradle";
		extensions["hs"] = "haskell";
		extensions["lhs"] = "haskell";
		extensions["haml"] = "haml";
		extensions["html"] = "html";
		extensions["htm"] = "html";
		extensions["shtml"] = "html";
		extensions["xhtml"] = "html";
		extensions["phtml"] = "html";
		extensions["tmpl"] = "html";
		extensions["tpl"] = "html";
		extensions["ctp"] = "html";
		extensions["jade"] = "jade";
		extensions["java"] = "java";
		extensions["jsp"] = "java";
		extensions["bsh"] = "java";
		extensions["js"] = "javascript";
		extensions["htc"] = "javascript";
		extensions["jsx"] = "javascript";
		extensions["jl"] = "julia";
		extensions["less"] = "less";
		extensions["lisp"] = "lisp";
		extensions["lua"] = "lua";
		extensions["md"] = "markdown";
		extensions["mdown"] = "markdown";
		extensions["markdown"] = "markdown";
		extensions["markdn"] = "markdown";
		extensions["m"] = "objective-c";
		extensions["mm"] = "objective-c++";
		extensions["M"] = "objective-c++";
		extensions["ml"] = "ocaml";
		extensions["mli"] = "ocaml";
		extensions["mll"] = "ocaml";
		extensions["mly"] = "ocaml";
		extensions["pl"] = "perl";
		extensions["pm"] = "perl";
		extensions["pod"] = "perl";
		extensions["t"] = "perl";
		extensions["cgi"] = "perl";
		extensions["fcgi"] = "perl";
		extensions["php"] = "php";
		extensions["inc"] = "php";
		extensions["pde"] = "processing";
		extensions["pug"] = "pug";
		extensions["pp"] = "puppet";
		extensions["py"] = "python";
		extensions["rpy"] = "python";
		extensions["pyw"] = "python";
		extensions["cpy"] = "python";
		extensions["r"] = "r";
		extensions["s"] = "r";
		extensions["rd"] = "r";
		extensions["Rprofile"] = "r";
		extensions["rkt"] = "racket";
		extensions["ss"] = "racket";
		extensions["scm"] = "racket";
		extensions["sch"] = "racket";
		extensions["rb"] = "ruby";
		extensions["rbx"] = "ruby";
		extensions["rjs"] = "ruby";
		extensions["Rakefile"] = "ruby";
		extensions["rake"] = "ruby";
		extensions["gemspec"] = "ruby";
		extensions["irbrc"] = "ruby";
		extensions["capfile"] = "ruby";
		extensions["Gemfile"] = "ruby";
		extensions["rb"] = "ruby";
		extensions["rs"] = "rust";
		extensions["rc"] = "rust";
		extensions["sass"] = "sass";
		extensions["scss"] = "sass";
		extensions["scala"] = "scala";
		extensions["sh"] = "shell-scripts";
		extensions["sql"] = "sql";
		extensions["styl"] = "stylus";
		extensions["stylus"] = "stylus";
		extensions["swift"] = "swift";
		extensions["tcl"] = "tcl";
		extensions["adp"] = "tcl";
		extensions["ts"] = "typescript";
		extensions["yaml"] = "yaml";
		extensions["yml"] = "yaml";

		QUrl url;
		url.setScheme( "dash-plugin" );
		url.setHost( "" );
		url.setQuery( "keys=" + keys[extensions[ext]] + "&query=" + text );
		QDesktopServices::openUrl( url );
	} else if ( text.length() > 0 ) {
		if ( ext.length() > 0 ) {
			ext += "%20";
		}
		QDesktopServices::openUrl( QUrl( "https://google.com/search?q=" + ext + text ) );
	}
}

void MainWindow::createFileMenu() {
	QMenu *fileMenu = new QMenu( tr( "&File" ), this );
	menuBar()->addMenu( fileMenu );

	fileMenu->addAction( tr( "&New File" ), this, SLOT( newFile() ), QKeySequence::New );
	fileMenu->addAction( tr( "&Open..." ), this, SLOT( openFile() ), QKeySequence::Open );

	mRecentFilesMenu = new QMenu( tr( "&Recent Files" ), fileMenu );
	fileMenu->addMenu( mRecentFilesMenu );

	fileMenu->addSeparator();

	mActionsRequiringFiles.append( fileMenu->addAction( tr( "&Save" ),
	                                                    this,
	                                                    SLOT(
								    saveFile() ),
	                                                    QKeySequence::Save ) );
#ifdef Q_OS_WIN
	mActionsRequiringFiles.append( fileMenu->addAction( tr( "Save &As..." ), this, SLOT( saveFileAs() ) ) );
	mActionsRequiringFiles.append( fileMenu->addAction( tr( "Save A&ll" ),
	                                                    this,
	                                                    SLOT( saveAllFiles() ),
	                                                    QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_S ) ) );
#else
	mActionsRequiringFiles.append( fileMenu->addAction( tr( "Save &As..." ),
	                                                    this,
	                                                    SLOT( saveFileAs() ),
	                                                    QKeySequence::SaveAs ) );
	mActionsRequiringFiles.append( fileMenu->addAction( tr( "Save A&ll" ), this, SLOT( saveAllFiles() ) ) );
#endif

	fileMenu->addSeparator();

	mActionsRequiringFiles.append( fileMenu->addAction( tr( "&Print..." ),
	                                                    this,
	                                                    SLOT( print() ),
	                                                    QKeySequence::Print ) );

	fileMenu->addSeparator();

	mActionsRequiringFiles.append( fileMenu->addAction( tr( "&Close" ),
	                                                    this,
	                                                    SLOT( closeFile() ),
	                                                    QKeySequence( Qt::CTRL + Qt::Key_W ) ) );
	mActionsRequiringFiles.append( fileMenu->addAction( tr( "Close All" ),
	                                                    this,
	                                                    SLOT( closeAllFiles() ),
	                                                    QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_W ) ) );
	mActionsRequiringFiles.append( fileMenu->addAction( tr( "Close All Except Current" ),
	                                                    this,
	                                                    SLOT( closeAllExceptCurrentFile() ) ) );

	fileMenu->addSeparator();

	fileMenu->addAction( tr( "E&xit" ), this, SLOT( close() ), QKeySequence::Quit );
}

void MainWindow::createEditMenu() {
	QMenu *editMenu = new QMenu( tr( "&Edit" ), this );
	menuBar()->addMenu( editMenu );

	mActionsRequiringFiles.append( editMenu->addAction( tr( "&Undo" ), this, SLOT( undo() ), QKeySequence::Undo ) );
	mActionsRequiringFiles.append( editMenu->addAction( tr( "&Redo" ), this, SLOT( redo() ), QKeySequence::Redo ) );

	editMenu->addSeparator();

	mActionsRequiringFiles.append( editMenu->addAction( tr( "&Cut" ), this, SLOT( cut() ), QKeySequence::Cut ) );
	mActionsRequiringFiles.append( editMenu->addAction( tr( "C&opy" ), this, SLOT( copy() ), QKeySequence::Copy ) );
	mActionsRequiringFiles.append( editMenu->addAction( tr( "&Paste" ),
	                                                    this,
	                                                    SLOT( paste() ),
	                                                    QKeySequence::Paste ) );

	editMenu->addSeparator();

	mActionsRequiringFiles.append( editMenu->addAction( tr( "Select &All" ),
	                                                    this,
	                                                    SLOT( selectAll() ),
	                                                    QKeySequence::SelectAll ) );

	editMenu->addSeparator();

	mActionsRequiringFiles.append( editMenu->addAction( tr( "&Find/Replace" ),
	                                                    gWindowManager,
	                                                    SLOT( showSearchBar() ),
	                                                    QKeySequence::Find ) );
	mActionsRequiringFiles.append( editMenu->addAction( "Find &Next",
	                                                    gWindowManager,
	                                                    SLOT( findNext() ),
	                                                    QKeySequence::FindNext ) );
	mActionsRequiringFiles.append( editMenu->addAction( "Find P&revious",
	                                                    gWindowManager,
	                                                    SLOT( findPrevious() ),
	                                                    QKeySequence::FindPrevious ) );
	editMenu->addAction( tr( "Advanced F&ind/Replace..." ),
	                     this,
	                     SLOT( showAdvancedSearch() ),
	                     QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_F ) );
#ifdef Q_OS_MAC
	mActionsRequiringFiles.append( editMenu->addAction( tr( "&Go To Line..." ),
	                                                    this,
	                                                    SLOT( showGotoLine() ),
	                                                    QKeySequence( Qt::CTRL + Qt::Key_L ) ) );
#else
	mActionsRequiringFiles.append( editMenu->addAction( tr( "&Go To Line..." ),
	                                                    this,
	                                                    SLOT( showGotoLine() ),
	                                                    QKeySequence( Qt::CTRL + Qt::Key_G ) ) );
#endif
}

void MainWindow::createViewMenu() {
	QMenu* viewMenu = new QMenu( tr( "&View" ), this );
	menuBar()->addMenu( viewMenu );

	mActionsRequiringFiles.append( viewMenu->addAction( tr( "&Actual Size" ),
	                                                    this,
	                                                    SLOT( resetZoom() ),
	                                                    QKeySequence( Qt::CTRL + Qt::Key_0 ) ) );

	QAction* zoomIn = viewMenu->addAction( tr( "Zoom &In" ), this, SLOT( zoomIn() ) );
	mActionsRequiringFiles.append( zoomIn );
	QList< QKeySequence > zoomInShortcuts;
	zoomInShortcuts.append( QKeySequence::ZoomIn );
	zoomInShortcuts.append( QKeySequence( Qt::CTRL + Qt::Key_Equal ) );
	zoomIn->setShortcuts( zoomInShortcuts );

	QAction* zoomOut = viewMenu->addAction( tr( "Zoom &Out" ), this, SLOT( zoomOut() ) );
	mActionsRequiringFiles.append( zoomOut );
	QList< QKeySequence > zoomOutShortcuts;
	zoomOutShortcuts.append( QKeySequence::ZoomOut );
	zoomOutShortcuts.append( QKeySequence( Qt::CTRL + Qt::Key_Underscore ) );
	zoomOut->setShortcuts( zoomOutShortcuts );

	viewMenu->addSeparator();

	mQuickListMenuItem = viewMenu->addAction( tr( "Quick File List" ), this, SLOT( switchToQuickList() ) );
	mQuickListMenuItem->setCheckable( true );

	mTabbedListMenuItem = viewMenu->addAction( tr( "Tabbed File List" ), this, SLOT( switchtoTabbedList() ) );
	mTabbedListMenuItem->setCheckable( true );

	if ( Options::FileListType == Options::QuickList ) {
		mQuickListMenuItem->setChecked( true );
	} else {
		mTabbedListMenuItem->setChecked( true );
	}

	viewMenu->addSeparator();

	mSyntaxMenu = new QMenu( tr( "&Syntax" ), viewMenu );
	viewMenu->addMenu( mSyntaxMenu );
	mSyntaxMenu->setEnabled( false );

	QAction* action = mSyntaxMenu->addAction( tr( "(No Highlighting)" ), this, SLOT( syntaxMenuOptionClicked() ) );
	action->setCheckable( true );
	mSyntaxMenuEntries.insert( QString(), action );

	QStringList categories = gSyntaxDefManager->getDefinitionCategories();
	categories.sort();
	foreach ( const QString &category, categories ) {
		QMenu* syntaxSubMenu = new QMenu( category, viewMenu );
		mSyntaxMenu->addMenu( syntaxSubMenu );

		QStringList syntaxes = gSyntaxDefManager->getSyntaxesInCategory( category );
		syntaxes.sort();
		foreach ( const QString &syntax, syntaxes ) {
			QAction* action = syntaxSubMenu->addAction( syntax, this, SLOT( syntaxMenuOptionClicked() ) );
			action->setData( syntax );
			action->setCheckable( true );
			mSyntaxMenuEntries.insert( syntax, action );
		}
	}

	viewMenu->addSeparator();
#ifdef Q_OS_MAC
	viewMenu->addAction( tr( "&Full Screen" ),
	                     this,
	                     SLOT( toggleFullScreen() ),
	                     QKeySequence( Qt::ALT + Qt::CTRL + Qt::Key_F ) );
#else
	viewMenu->addAction( tr( "&Full Screen" ), this, SLOT( toggleFullScreen() ), QKeySequence( Qt::Key_F11 ) );
#endif
}

void MainWindow::createToolsMenu() {
	QMenu *toolsMenu = new QMenu( tr( "&Tools" ), this );
	menuBar()->addMenu( toolsMenu );

	toolsMenu->addAction( tr( "&Regular Expression Tester..." ), gWindowManager, SLOT( showRegExpTester() ) );
	toolsMenu->addAction( tr( "&HTML Preview..." ), this, SLOT( showHTMLPreview() ) );
	toolsMenu->addAction( tr( "&Options..." ), this, SLOT( options() ), QKeySequence::Preferences );
}

void MainWindow::createWindowMenu() {
	QMenu *windowMenu = new QMenu( tr( "&Window" ), this );
	menuBar()->addMenu( windowMenu );

#ifdef Q_OS_MAC
	mActionsRequiringFiles.append( windowMenu->addAction( tr( "&Previous Window" ),
	                                                      gWindowManager,
	                                                      SLOT( previousWindow() ),
	                                                      QKeySequence::PreviousChild ) );
#else
	mActionsRequiringFiles.append( windowMenu->addAction( tr( "&Previous Window" ),
	                                                      gWindowManager,
	                                                      SLOT( previousWindow() ),
	                                                      QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_Tab ) ) );
#endif
	mActionsRequiringFiles.append( windowMenu->addAction( tr( "&Next Window" ),
	                                                      gWindowManager,
	                                                      SLOT( nextWindow() ),
	                                                      QKeySequence::NextChild ) );

	windowMenu->addSeparator();

	mActionsRequiringFiles.append( windowMenu->addAction( tr( "Split View &Vertically" ),
	                                                      gWindowManager,
	                                                      SLOT( splitVertically() ),
	                                                      QKeySequence( Qt::CTRL + Qt::Key_E, Qt::Key_3 ) ) );
	mActionsRequiringFiles.append( windowMenu->addAction( tr( "Split View &Horizontally" ),
	                                                      gWindowManager,
	                                                      SLOT( splitHorizontally() ),
	                                                      QKeySequence( Qt::CTRL + Qt::Key_E, Qt::Key_2 ) ) );
	mActionsRequiringSplitViews.append( windowMenu->addAction( tr( "&Remove Current Split" ),
	                                                           gWindowManager,
	                                                           SLOT( removeSplit() ),
	                                                           QKeySequence( Qt::CTRL + Qt::Key_E, Qt::Key_1 ) ) );
	mActionsRequiringSplitViews.append( windowMenu->addAction( tr( "Remove &All Splits" ),
	                                                           gWindowManager,
	                                                           SLOT( removeAllSplits() ),
	                                                           QKeySequence( Qt::CTRL + Qt::Key_E, Qt::Key_0 ) ) );

	windowMenu->addSeparator();

	mActionsRequiringSplitViews.append( windowMenu->addAction( tr( "Ne&xt Split Panel" ),
	                                                           gWindowManager,
	                                                           SLOT( nextSplit() ),
	                                                           QKeySequence( Qt::ALT + Qt::Key_Right ) ) );
	mActionsRequiringSplitViews.append( windowMenu->addAction( tr( "Pre&vious Split Panel" ),
	                                                           gWindowManager,
	                                                           SLOT( previousSplit() ),
	                                                           QKeySequence( Qt::ALT + Qt::Key_Left ) ) );
}

void MainWindow::createHelpMenu() {
	QMenu *helpMenu = new QMenu( tr( "&Help" ), this );
	menuBar()->addMenu( helpMenu );

	helpMenu->addAction( tr( "Context Help" ), this, SLOT( contextHelp() ), QKeySequence( Qt::Key_F1 ) );

	helpMenu->addAction( tr( "&About" ), this, SLOT( about() ) );

	QAction *updates = new QAction( tr( "&Check for updates..." ), this );
	updates->setMenuRole( QAction::ApplicationSpecificRole );
	connect( updates, SIGNAL( triggered() ), this, SLOT( checkForUpdates() ) );

	helpMenu->addAction( updates );
}

void MainWindow::createMacDockMenu() {
#ifndef Q_OS_MAC
	return;
#else
	QMenu *dockMenu = new QMenu( this );

	dockMenu->addAction( tr( "New File" ), this, SLOT( newFile() ) );

	qt_mac_set_dock_menu( dockMenu );
#endif
}

void MainWindow::createShortcuts() {
#ifdef Q_OS_MAC
	QAction *deleteLine = new QAction( this );

	deleteLine->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_Backspace ) );
	connect( deleteLine, SIGNAL( triggered() ), this, SLOT( deleteLine() ) );

	addAction( deleteLine );
#endif
}

void MainWindow::showErrorMessage( QString error ) {
	QMessageBox::critical( this, "Error", error );
}

void MainWindow::showStatusMessage( QString message ) {
	mStatusLine->setText( message );
}

void MainWindow::undo() {
	QMetaObject::invokeMethod( QApplication::focusWidget(), "undo" );
}

void MainWindow::redo() {
	QMetaObject::invokeMethod( QApplication::focusWidget(), "redo" );
}

void MainWindow::cut() {
	QMetaObject::invokeMethod( QApplication::focusWidget(), "cut" );
}

void MainWindow::copy() {
	QMetaObject::invokeMethod( QApplication::focusWidget(), "copy" );
}

void MainWindow::paste() {
	QMetaObject::invokeMethod( QApplication::focusWidget(), "paste" );
}

void MainWindow::selectAll() {
	QMetaObject::invokeMethod( QApplication::focusWidget(), "selectAll" );
}

void MainWindow::deleteLine() {
	Editor* current = gWindowManager->currentEditor();
	if ( current ) {
		current->deleteLine();
	}
}

void MainWindow::showGotoLine() {
	GotoLineDialog dlg( this );
	if ( dlg.exec() ) {
		Editor* current = gWindowManager->currentEditor();
		if ( current ) {
			current->gotoLine( dlg.lineNumber() );
		}
	}
}

void MainWindow::showAdvancedSearch() {
	AdvancedSearchDialog dlg( this );

	dlg.exec();
}

void MainWindow::closeEvent( QCloseEvent* event ) {
	nextStartupPrompt();
	Tools::saveCurrentFiles();

	if ( ! gOpenFileManager.closeAllFiles() ) {
		event->ignore();
		return;
	}

	// Save the geometry and toolbar state of this window on the way out
	QSettings settings;
	settings.setValue( "mainwindow/geometry", saveGeometry() );
	settings.setValue( "mainwindow/state", saveState() );
	QMainWindow::closeEvent( event );
}

void MainWindow::nextStartupPrompt() {
	if ( ! Options::ShutdownPrompt ) {
		return;
	}

	if ( gOpenFileManager.getFileCount() == 0 ) {
		return;
	}

	ShutdownPrompt dlg( this );
	dlg.exec();
}

void MainWindow::syntaxMenuOptionClicked() {
	QObject* eventSource = QObject::sender();
	QAction* action = static_cast< QAction* >( eventSource );
	QString syntaxName = action->data().toString();

	Editor* currentEditor = getCurrentEditor();
	if ( ! currentEditor ) {
		return;
	}
	currentEditor->getFile()->setSyntax( syntaxName );
}

Editor* MainWindow::getCurrentEditor() {
	return gWindowManager->currentEditor();
}

void MainWindow::currentEditorChanged() {
	updateSyntaxSelection();
}

void MainWindow::updateSyntaxSelection() {
	if ( mCurrentSyntaxMenuItem != NULL ) {
		mCurrentSyntaxMenuItem->setChecked( false );
		mCurrentSyntaxMenuItem = NULL;
	}

	Editor* editor = getCurrentEditor();
	if ( editor ) {
		mSyntaxMenu->setEnabled( true );
		BaseFile* file = editor->getFile();
		QString syntaxName = file->getSyntax();
		mCurrentSyntaxMenuItem = mSyntaxMenuEntries.value( syntaxName, NULL );
		if ( mCurrentSyntaxMenuItem != NULL ) {
			mCurrentSyntaxMenuItem->setChecked( true );
		}
	} else {
		mSyntaxMenu->setEnabled( false );
	}
}

void MainWindow::updateRecentFilesMenu() {
	mRecentFilesMenu->clear();

	for ( int ii = 0; ii < mRecentFiles.length(); ii++ ) {
		QAction* action =
			mRecentFilesMenu->addAction( mRecentFiles[ii].getDisplayPath(),
			                             this,
			                             SLOT( openSingleFile() ) );
		action->setData( ii );
	}
}

void MainWindow::addRecentFile( Location loc ) {
	if ( loc.isNull() ) {
		return;
	}

	for ( int ii = 0; ii < mRecentFiles.length(); ii++ ) {
		if ( loc.getDisplayPath() == mRecentFiles[ii].getDisplayPath() ) {
			mRecentFiles.removeAt( ii );
			ii--;
		}
	}
	mRecentFiles.push_front( loc );

	// Only keep the 10 most recent files
	for ( int ii = 9; ii < mRecentFiles.length(); ii++ ) {
		mRecentFiles.removeAt( ii );
	}

	updateRecentFilesMenu();

	Tools::saveRecentFiles( mRecentFiles );
}

void MainWindow::checkForUpdates() {
	gSiteManager->checkForUpdates( true );
}

void MainWindow::dragEnterEvent( QDragEnterEvent *event ) {
	if ( event->mimeData()->hasFormat( "text/uri-list" ) ) {
		event->acceptProposedAction();
	}
}

void MainWindow::dropEvent( QDropEvent *event ) {
	QList< QUrl > urls = event->mimeData()->urls();
	if ( urls.isEmpty() ) {
		return;
	}

	QString fileName = urls.first().toLocalFile();
	if ( fileName.isEmpty() ) {
		return;
	}

	openSingleFile( Location( fileName ) );
}

void MainWindow::showHTMLPreview() {
	HTMLPreview* htmlPreview = new HTMLPreview( this );
	QDockWidget* htmlWrapper = new QDockWidget( "HTML Preview", 0 );

	connect( htmlWrapper, SIGNAL( visibilityChanged( bool ) ), this, SLOT( closeHTMLPreview( bool ) ) );

	htmlWrapper->setFeatures( QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable );
	htmlWrapper->setWidget( htmlPreview );
	addDockWidget( Qt::BottomDockWidgetArea, htmlWrapper, Qt::Horizontal );

	htmlWrapper->show();

	htmlWrapper->setObjectName( tr( "HTML Preview" ) );
}

void MainWindow::closeHTMLPreview( bool visible ) {
	if ( visible ) {
		return;
	}

	QObject* eventSource = QObject::sender();
	QDockWidget* htmlWrapper = static_cast< QDockWidget* >( eventSource );

	// HTMLPreview* htmlPreview = (HTMLPreview*)htmlWrapper->widget();

	// delete htmlPreview;
	htmlWrapper->deleteLater();
}

void MainWindow::resetZoom() {
	Options::EditorFontZoom = 100;

	gDispatcher->emitOptionsChanged();
}

void MainWindow::zoomIn() {
	Options::EditorFontZoom += 10;

	gDispatcher->emitOptionsChanged();
}

void MainWindow::zoomOut() {
	Options::EditorFontZoom -= 10;
	if ( Options::EditorFontZoom < 10 ) {
		Options::EditorFontZoom = 10;
	}

	gDispatcher->emitOptionsChanged();
}

void MainWindow::print() {
	Editor* current = getCurrentEditor();
	if ( ! current ) {
		return;
	}

	QPrinter printer;

	QPrintDialog *dialog = new QPrintDialog( &printer, this );
	dialog->setWindowTitle( tr( "Print Document" ) );

	if ( dialog->exec() != QDialog::Accepted ) {
		return;
	}

	current->print( &printer );
}

void MainWindow::toggleFullScreen() {
	if ( isFullScreen() ) {
		if ( mWasMaximized ) {
			showMaximized();
		} else {
			showNormal();
		}
	} else {
		if ( isMaximized() ) {
			mWasMaximized = true;
		} else {
			mWasMaximized = false;
		}
		showFullScreen();
	}
}

// Override for QMainWindow::createPopupMenu. Removes menu entries for things I don't want shown.
QMenu* MainWindow::createPopupMenu() {
	QMenu* menu = new QMenu( this );

	foreach ( QDockWidget * dockWidget, mMenuControlledDockWidgets ) {
		menu->addAction( dockWidget->toggleViewAction() );
	}

	menu->addSeparator();

	foreach ( QToolBar * toolbar, mMenuControlledToolBar ) {
		menu->addAction( toolbar->toggleViewAction() );
	}

	return menu;
}

void MainWindow::switchToQuickList() {
	mFileList->setVisible( true );
	mQuickListMenuItem->setChecked( true );

	mTabbedFileList->setVisible( false );
	mTabbedListMenuItem->setChecked( false );

	Options::FileListType = Options::QuickList;
}

void MainWindow::switchtoTabbedList() {
	mTabbedFileList->setVisible( true );
	mTabbedListMenuItem->setChecked( true );

	mFileList->setVisible( false );
	mQuickListMenuItem->setChecked( false );

	Options::FileListType = Options::TabbedList;
}

void MainWindow::openFileListChanged() {
	// Enable / disable all actions that are dependant on open files
	bool filesOpen = ( gOpenFileManager.getFileCount() > 0 );
	foreach ( QAction * action, mActionsRequiringFiles ) {
		action->setEnabled( filesOpen );
	}
}

void MainWindow::viewSplittingChanged() {
	// Enable / disable all actions that are dependant on split views
	bool viewSplit = gWindowManager->isSplit();
	foreach ( QAction * action, mActionsRequiringSplitViews ) {
		action->setEnabled( viewSplit );
	}
}
