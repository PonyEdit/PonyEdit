#include "main/mainwindow.h"

#include <QDebug>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QRegExp>
#include <QTime>
#include <QTimer>
#include <QCryptographicHash>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QCoreApplication>
#include <QApplication>
#include <QSettings>
#include <QErrorMessage>
#include <QPrintDialog>
#include <QPrinter>

#include "file/filedialog.h"
#include "file/filelist.h"
#include "file/tabbedfilelist.h"
#include "file/openfilemanager.h"
#include "editor/editor.h"
#include "options/options.h"
#include "options/optionsdialog.h"
#include "main/globaldispatcher.h"
#include "tools.h"
#include "gotolinedialog.h"
#include "advancedsearchdialog.h"
#include "file/unsavedchangesdialog.h"
#include "file/openfilemanager.h"
#include "syntax/syntaxdefmanager.h"
#include "ssh/connectionstatuspane.h"
#include "website/sitemanager.h"
#include "aboutdialog.h"
#include "tools/htmlpreview.h"
#include "licence/licence.h"
#include "licence/licencecheckdialog.h"
#include "shutdownprompt.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	mUnsavedChangesDialog = NULL;
	mCurrentSyntaxMenuItem = NULL;

	setWindowTitle(tr("PonyEdit"));

	setAcceptDrops(true);

	gWindowManager = new WindowManager(this);
	gWindowManager->setMinimumWidth(200);
	setCentralWidget(gWindowManager);

	mFileList = new FileList();
	addDockWidget(Qt::LeftDockWidgetArea, mFileList, Qt::Vertical);
	mFileList->setObjectName("File List");

	mTabbedFileList = new TabbedFileList();
	addDockWidget(Qt::TopDockWidgetArea, mTabbedFileList, Qt::Horizontal);
	mTabbedFileList->setObjectName("Tabbed File List");

	if(Options::FileListType == Options::QuickList)
	{
		mFileList->setVisible(true);
		mTabbedFileList->setVisible(false);
	}
	else
	{
		mFileList->setVisible(false);
		mTabbedFileList->setVisible(true);
	}

	mConnectionStatusPane = new ConnectionStatusPane();
	addDockWidget(Qt::LeftDockWidgetArea, mConnectionStatusPane, Qt::Vertical);
	mConnectionStatusPane->setObjectName("Dropped Connections");

	mStatusLine = new QLabel();
	mStatusBar = new QStatusBar();
	mStatusBar->addPermanentWidget(mStatusLine);
	setStatusBar(mStatusBar);

	createToolbar();

	createFileMenu();
	createEditMenu();
	createViewMenu();
	createToolsMenu();
	createWindowMenu();
	createHelpMenu();
	createMacDockMenu();

	connect(gDispatcher, SIGNAL(generalErrorMessage(QString)), this, SLOT(showErrorMessage(QString)), Qt::QueuedConnection);
	connect(gDispatcher, SIGNAL(generalStatusMessage(QString)), this, SLOT(showStatusMessage(QString)), Qt::QueuedConnection);
	connect(gDispatcher, SIGNAL(selectFile(BaseFile*)), this, SLOT(fileSelected(BaseFile*)));
	connect(gWindowManager, SIGNAL(currentChanged()), this, SLOT(currentEditorChanged()), Qt::QueuedConnection);
	connect(gDispatcher, SIGNAL(syntaxChanged(BaseFile*)), this, SLOT(updateSyntaxSelection()));
	connect(&gOpenFileManager, SIGNAL(fileOpened(BaseFile*)), this, SLOT(openFileListChanged()));
	connect(&gOpenFileManager, SIGNAL(fileClosed(BaseFile*)), this, SLOT(openFileListChanged()));
	connect(gWindowManager, SIGNAL(splitChanged()), this, SLOT(viewSplittingChanged()));

	mRecentFiles = Tools::loadRecentFiles();
	updateRecentFilesMenu();

	/*const char* key = "dGhpbmdhbG9uOjE6MjAxMS0wNi0wMToxLwCZAmzUPfc1ZgGCzxFju78l4b7KAiXHdWsvjIyJ3hh+VeI0ftUXBaXMe21c0Z6YygAbO7qBDRik1cjFBQiTiR8a1jogP1Uw5nsK5lLAvcTRxF3QYnmb9DakiF11Bi6NO0wuYxIPsyF52m3K9TFfdX1UF28ayzhWXiiJBDI9ykOtPzNLbA3UANpO0ibT3kfJzhGOHsU5Nyj+43aHizid3rTyduTtMKUsTjX8WkQ3l9xDltLVxiNYiU6blSemNmP+xUmC2G7VQ7XmyJ7KQUlGNLW+rYT5Vpt7SYD7/QReFGB7xRH2PrZTbHktH9irO78tHWAUNeAFNhaUTD0GWo9ZVz8=";
	Licence l(key);
	qDebug() << "Licence is valid: " << l.isValid();
	qDebug() << "Licence has expired: " << l.hasExpired();
	qDebug() << "Licence expiry date: " << l.getExpiry();
	qDebug() << "Licence login name: " << l.getLogin();
	qDebug() << "Licence max version: " << l.getMaximumVersion();
	qDebug() << "Licence issue id: " << l.getIssueId();*/

	restoreState();

	QTimer::singleShot(0, this, SLOT(checkLicence()));

	openFileListChanged();
	viewSplittingChanged();
}

MainWindow::~MainWindow()
{
	delete gWindowManager;
}

void MainWindow::restoreState()
{
	QSettings settings;
	restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
	QMainWindow::restoreState(settings.value("mainwindow/state").toByteArray());
	mConnectionStatusPane->hide();
}

void MainWindow::createToolbar()
{
	//
	//	Main toolbar
	//

	QToolBar* toolbar = new QToolBar("File Toolbar");
	toolbar->addAction(QIcon(":/icons/new.png"), "New", this, SLOT(newFile()));
	toolbar->addAction(QIcon(":/icons/open.png"), "Open", this, SLOT(openFile()));
	mActionsRequiringFiles.append(toolbar->addAction(QIcon(":/icons/save.png"), "Save", this, SLOT(saveFile())));
	toolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	addToolBar(toolbar);
	registerContextMenuItem(toolbar);
	toolbar->setObjectName("File Toolbar");

	QWidget* spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	toolbar->addWidget(spacer);

	//
	//	View toolbar
	//

	toolbar = new QToolBar("View Toolbar");
	mActionsRequiringFiles.append(toolbar->addAction(QIcon(":/icons/verticalsplit.png"), "Split View Vertically", gWindowManager, SLOT(splitVertically())));
	mActionsRequiringFiles.append(toolbar->addAction(QIcon(":/icons/horizontalsplit.png"), "Split View Horizontally", gWindowManager, SLOT(splitHorizontally())));
	mActionsRequiringSplitViews.append(toolbar->addAction(QIcon(":/icons/removesplit.png"), "Remove Split", gWindowManager, SLOT(removeSplit())));

	addToolBar(toolbar);
	registerContextMenuItem(toolbar);
	toolbar->setObjectName("View Toolbar");

	//
	//	Trial time left toolbar
	//

	mTrialRemainingBar = new QToolBar("Time Remaining");
	mTrialRemainingButton = new QToolButton(mTrialRemainingBar);
	connect(mTrialRemainingButton, SIGNAL(clicked()), this, SLOT(showLicenceDialog()));
	addToolBar(mTrialRemainingBar);
	mTrialRemainingBar->addWidget(mTrialRemainingButton);
	mTrialRemainingBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	//
	//	Feedback toolbar
	//

	QToolBar* feedbackToolbar = new QToolBar("Feedback Toolbar");
	feedbackToolbar->setObjectName("Feedback Toolbar");
	feedbackToolbar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	QToolButton* feedbackButton = new QToolButton(feedbackToolbar);
	feedbackButton->setText(tr("Feedback") + " ");

	QMenu *feedbackMenu = new QMenu(toolbar);

	feedbackMenu->addAction(tr("PonyEdit Made Me Happy Because..."), gSiteManager, SLOT(feedbackHappy()));
	feedbackMenu->addAction(tr("PonyEdit Made Me Sad Because..."), gSiteManager, SLOT(feedbackSad()));

	feedbackButton->setMenu(feedbackMenu);

	feedbackButton->setPopupMode(QToolButton::InstantPopup);

	feedbackToolbar->addWidget(feedbackButton);

	addToolBar(feedbackToolbar);
	registerContextMenuItem(feedbackToolbar);
}

void MainWindow::newFile()
{
	QString path = "";
	Location location(path);
	BaseFile* file = location.getFile();

	gWindowManager->displayFile(file);

	gDispatcher->emitSelectFile(file);
}

void MainWindow::openFile()
{
	FileDialog dlg(this);
	if (dlg.exec())
	{
		QList<Location> locations = dlg.getSelectedLocations();
		if(locations.length() > 20)
		{
			QMessageBox msgBox;
			msgBox.setWindowTitle(tr("Opening Many Files"));
			msgBox.setText(tr("You have selected %1 files to open.").arg(locations.length()));
			msgBox.setInformativeText(tr("This may take some time to complete. Are you sure you want to do this?"));
			msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			msgBox.setDefaultButton(QMessageBox::Yes);

			if(msgBox.exec() == QMessageBox::No)
				return;
		}
		foreach (Location location, locations)
			openSingleFile(location);
	}
}

void MainWindow::openSingleFile()
{
	QAction* action = static_cast<QAction*>(sender());
	if(action)
		openSingleFile(mRecentFiles[action->data().toInt()]);
}

void MainWindow::openSingleFile(const Location& loc)
{
	if (!loc.isDirectory())
	{
		BaseFile* file;
		try
		{
			file = Location(loc).getFile();
		}
		catch(QString &e)
		{
			qDebug() << e;
			return;
		}

		if (file->isClosed())
			file->open();

		gWindowManager->displayFile(file);

		gDispatcher->emitSelectFile(file);

		addRecentFile(loc);

		connect(file, SIGNAL(openStatusChanged(int)), this, SLOT(updateTitle()));
		connect(file, SIGNAL(unsavedStatusChanged()), this, SLOT(updateTitle()));
	}
}

void MainWindow::saveFile()
{
	Editor* current = gWindowManager->currentEditor();
	if (current)
	{
		if(current->getFile()->getLocation().getProtocol() == Location::Unsaved)
			saveFileAs();
		else
		{
			try
			{
				current->save();
			}
			catch(QString &e)
			{
				qDebug() << e;
			}
		}
	}
}

void MainWindow::saveFileAs()
{
	Editor* current = gWindowManager->currentEditor();
	if (current == NULL)
		return;

	FileDialog dlg(this, true);
	if(dlg.exec())
	{
		Location loc = dlg.getNewLocation();
		try
		{
			loc.getFile()->newFile(current->getFile()->getContent());
		}
		catch(QString &e)
		{
			qDebug() << e;
			return;
		}

		loc.getFile()->open();

		openSingleFile(loc);

		if(current->getFile()->getLocation().getProtocol() == Location::Unsaved)
			current->close();
	}
}

void MainWindow::saveAllFiles()
{
	QList<BaseFile*> unsavedFiles = gOpenFileManager.getUnsavedFiles(gOpenFileManager.getOpenFiles());

	foreach(BaseFile* file, unsavedFiles)
		file->save();
}

void MainWindow::closeFile()
{
	Editor* current = gWindowManager->currentEditor();
	if (current)
	{
		QList<BaseFile *> files;
		files.append(current->getFile());

		gOpenFileManager.closeFiles(files);
	}
}

void MainWindow::closeAllFiles()
{
	gOpenFileManager.closeAllFiles();
}

void MainWindow::closeAllExceptCurrentFile()
{
	QList<BaseFile*> openFiles = gOpenFileManager.getOpenFiles();

	BaseFile* current = gWindowManager->currentEditor()->getFile();

	foreach(BaseFile* file, openFiles)
		if(file != current)
			file->close();
}

void MainWindow::fileSelected(BaseFile* file)
{
	if (!file) return;

	updateTitle(file);

	gWindowManager->displayFile(file);
}

void MainWindow::updateTitle()
{
	BaseFile* file = static_cast<BaseFile*>(sender());
	if(file->isClosed())
	{
		setWindowTitle("PonyEdit");
		return;
	}
	QList<Editor*> editors = file->getAttachedEditors();

	bool current = false;
	foreach(Editor* editor, editors)
	{
		if(editor == gWindowManager->currentEditor())
			current = true;
	}

	if(current)
		updateTitle(file);
}

void MainWindow::updateTitle(BaseFile* file)
{
	bool modified = false;
	QString title = "PonyEdit - ";
	title += file->getLocation().getLabel();
	if(file->hasUnsavedChanges())
		modified = true;

#ifdef Q_OS_MAC
	if(file->getLocation().getProtocol() == Location::Local)
		setWindowFilePath(file->getLocation().getPath());
	else
	{
		setWindowFilePath("");
		QIcon icon = file->getLocation().getIcon();
		setWindowIcon(icon);
	}
#endif

	setWindowTitle(title);
	setWindowModified(modified);
}

void MainWindow::options()
{
	OptionsDialog dlg(this);
	dlg.exec();
}

void MainWindow::about()
{
	AboutDialog dlg(this);
	dlg.exec();
}

void MainWindow::createFileMenu()
{
	QMenu *fileMenu = new QMenu(tr("&File"), this);
	menuBar()->addMenu(fileMenu);

	fileMenu->addAction(tr("&New File"), this, SLOT(newFile()), QKeySequence::New);
	fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()), QKeySequence::Open);

	mRecentFilesMenu = new QMenu(tr("&Recent Files"), fileMenu);
	fileMenu->addMenu(mRecentFilesMenu);

	fileMenu->addSeparator();

	mActionsRequiringFiles.append(fileMenu->addAction(tr("&Save"), this, SLOT(saveFile()), QKeySequence::Save));
#ifdef Q_OS_WIN
	mActionsRequiringFiles.append(fileMenu->addAction(tr("Save &As..."), this, SLOT(saveFileAs())));
	mActionsRequiringFiles.append(fileMenu->addAction(tr("Save A&ll"), this, SLOT(saveAllFiles()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S)));
#else
	mActionsRequiringFiles.append(fileMenu->addAction(tr("Save &As..."), this, SLOT(saveFileAs()), QKeySequence::SaveAs));
	mActionsRequiringFiles.append(fileMenu->addAction(tr("Save A&ll"), this, SLOT(saveAllFiles())));
#endif

	fileMenu->addSeparator();

	mActionsRequiringFiles.append(fileMenu->addAction(tr("&Print..."), this, SLOT(print()), QKeySequence::Print));

	fileMenu->addSeparator();

	mActionsRequiringFiles.append(fileMenu->addAction(tr("&Close"), this, SLOT(closeFile()), QKeySequence(Qt::CTRL + Qt::Key_W)));
	mActionsRequiringFiles.append(fileMenu->addAction(tr("Close All"), this, SLOT(closeAllFiles()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_W)));
	mActionsRequiringFiles.append(fileMenu->addAction(tr("Close All Except Current"), this, SLOT(closeAllExceptCurrentFile())));

	fileMenu->addSeparator();

	fileMenu->addAction(tr("E&xit"), this, SLOT(close()), QKeySequence::Quit);
}

void MainWindow::createEditMenu()
{
	QMenu *editMenu = new QMenu(tr("&Edit"), this);
	menuBar()->addMenu(editMenu);

	mActionsRequiringFiles.append(editMenu->addAction(tr("&Undo"), this, SLOT(undo()), QKeySequence::Undo));
	mActionsRequiringFiles.append(editMenu->addAction(tr("&Redo"), this, SLOT(redo()), QKeySequence::Redo));

	editMenu->addSeparator();

	mActionsRequiringFiles.append(editMenu->addAction(tr("&Cut"), this, SLOT(cut()), QKeySequence::Cut));
	mActionsRequiringFiles.append(editMenu->addAction(tr("C&opy"), this, SLOT(copy()), QKeySequence::Copy));
	mActionsRequiringFiles.append(editMenu->addAction(tr("&Paste"), this, SLOT(paste()), QKeySequence::Paste));

	editMenu->addSeparator();

	mActionsRequiringFiles.append(editMenu->addAction(tr("Select &All"), this, SLOT(selectAll()), QKeySequence::SelectAll));

	editMenu->addSeparator();

	mActionsRequiringFiles.append(editMenu->addAction(tr("&Find/Replace"), gWindowManager, SLOT(showSearchBar()), QKeySequence::Find));
	mActionsRequiringFiles.append(editMenu->addAction("Find &Next", gWindowManager, SLOT(findNext()), QKeySequence::FindNext));
	mActionsRequiringFiles.append(editMenu->addAction("Find P&revious", gWindowManager, SLOT(findPrevious()), QKeySequence::FindPrevious));
	editMenu->addAction(tr("Advanced F&ind/Replace..."), this, SLOT(showAdvancedSearch()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
#ifdef Q_OS_MAC
	mActionsRequiringFiles.append(editMenu->addAction(tr("&Go To Line..."), this, SLOT(showGotoLine()), QKeySequence(Qt::CTRL + Qt::Key_L)));
#else
	mActionsRequiringFiles.append(editMenu->addAction(tr("&Go To Line..."), this, SLOT(showGotoLine()), QKeySequence(Qt::CTRL + Qt::Key_G)));
#endif
}

void MainWindow::createViewMenu()
{
	QMenu* viewMenu = new QMenu(tr("&View"), this);
	menuBar()->addMenu(viewMenu);

	mActionsRequiringFiles.append(viewMenu->addAction(tr("&Actual Size"), this, SLOT(resetZoom()), QKeySequence(Qt::CTRL + Qt::Key_0)));

	QAction* zoomIn = viewMenu->addAction(tr("Zoom &In"), this, SLOT(zoomIn()));
	mActionsRequiringFiles.append(zoomIn);
	QList<QKeySequence> zoomInShortcuts;
	zoomInShortcuts.append(QKeySequence::ZoomIn);
	zoomInShortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Equal));
	zoomIn->setShortcuts(zoomInShortcuts);

	QAction* zoomOut = viewMenu->addAction(tr("Zoom &Out"), this, SLOT(zoomOut()));
	mActionsRequiringFiles.append(zoomOut);
	QList<QKeySequence> zoomOutShortcuts;
	zoomOutShortcuts.append(QKeySequence::ZoomOut);
	zoomOutShortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_Underscore));
	zoomOut->setShortcuts(zoomOutShortcuts);

	viewMenu->addSeparator();

	mQuickListMenuItem = viewMenu->addAction(tr("Quick File List"), this, SLOT(switchToQuickList()));
	mQuickListMenuItem->setCheckable(true);

	mTabbedListMenuItem = viewMenu->addAction(tr("Tabbed File List"), this, SLOT(switchtoTabbedList()));
	mTabbedListMenuItem->setCheckable(true);

	if(Options::FileListType == Options::QuickList)
		mQuickListMenuItem->setChecked(true);
	else
		mTabbedListMenuItem->setChecked(true);

	viewMenu->addSeparator();

	mSyntaxMenu = new QMenu(tr("&Syntax"), viewMenu);
	viewMenu->addMenu(mSyntaxMenu);
	mSyntaxMenu->setEnabled(false);

	QAction* action = mSyntaxMenu->addAction(tr("(No Highlighting)"), this, SLOT(syntaxMenuOptionClicked()));
	action->setCheckable(true);
	mSyntaxMenuEntries.insert(QString(), action);

	QStringList categories = gSyntaxDefManager->getDefinitionCategories();
	categories.sort();
	foreach (const QString& category, categories)
	{
		QMenu* syntaxSubMenu = new QMenu(category, viewMenu);
		mSyntaxMenu->addMenu(syntaxSubMenu);

		QStringList syntaxes = gSyntaxDefManager->getSyntaxesInCategory(category);
		syntaxes.sort();
		foreach (const QString& syntax, syntaxes)
		{
			QAction* action = syntaxSubMenu->addAction(syntax, this, SLOT(syntaxMenuOptionClicked()));
			action->setData(syntax);
			action->setCheckable(true);
			mSyntaxMenuEntries.insert(syntax, action);
		}
	}

	viewMenu->addSeparator();
#ifdef Q_OS_MAC
	viewMenu->addAction(tr("&Full Screen"), this, SLOT(toggleFullScreen()), QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_F));
#else
	viewMenu->addAction(tr("&Full Screen"), this, SLOT(toggleFullScreen()), QKeySequence(Qt::Key_F11));
#endif
}

void MainWindow::createToolsMenu()
{
	QMenu *toolsMenu = new QMenu(tr("&Tools"), this);
	menuBar()->addMenu(toolsMenu);

	toolsMenu->addAction(tr("&Regular Expression Tester..."), gWindowManager, SLOT(showRegExpTester()));
	toolsMenu->addAction(tr("&HTML Preview..."), this, SLOT(showHTMLPreview()));
	toolsMenu->addAction(tr("&Options..."), this, SLOT(options()), QKeySequence::Preferences);
}

void MainWindow::createWindowMenu()
{
	QMenu *windowMenu = new QMenu(tr("&Window"), this);
	menuBar()->addMenu(windowMenu);

#ifdef Q_OS_MAC
	mActionsRequiringFiles.append(windowMenu->addAction(tr("&Previous Window"), gWindowManager, SLOT(previousWindow()), QKeySequence::PreviousChild));
#else
	mActionsRequiringFiles.append(windowMenu->addAction(tr("&Previous Window"), gWindowManager, SLOT(previousWindow()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab)));
#endif
	mActionsRequiringFiles.append(windowMenu->addAction(tr("&Next Window"), gWindowManager, SLOT(nextWindow()), QKeySequence::NextChild));

	windowMenu->addSeparator();

	mActionsRequiringFiles.append(windowMenu->addAction(tr("Split View &Vertically"), gWindowManager, SLOT(splitVertically()), QKeySequence(Qt::CTRL + Qt::Key_E, Qt::Key_3)));
	mActionsRequiringFiles.append(windowMenu->addAction(tr("Split View &Horizontally"), gWindowManager, SLOT(splitHorizontally()), QKeySequence(Qt::CTRL + Qt::Key_E, Qt::Key_2)));
	mActionsRequiringSplitViews.append(windowMenu->addAction(tr("&Remove Current Split"), gWindowManager, SLOT(removeSplit()), QKeySequence(Qt::CTRL + Qt::Key_E, Qt::Key_1)));
	mActionsRequiringSplitViews.append(windowMenu->addAction(tr("Remove &All Splits"), gWindowManager, SLOT(removeAllSplits()), QKeySequence(Qt::CTRL + Qt::Key_E, Qt::Key_0)));

	windowMenu->addSeparator();

	mActionsRequiringSplitViews.append(windowMenu->addAction(tr("Ne&xt Split Panel"), gWindowManager, SLOT(nextSplit()), QKeySequence(Qt::ALT + Qt::Key_Right)));
	mActionsRequiringSplitViews.append(windowMenu->addAction(tr("Pre&vious Split Panel"), gWindowManager, SLOT(previousSplit()), QKeySequence(Qt::ALT + Qt::Key_Left)));
}

void MainWindow::createHelpMenu()
{
	 QMenu *helpMenu = new QMenu(tr("&Help"), this);
	 menuBar()->addMenu(helpMenu);

	 helpMenu->addAction(tr("&About"), this, SLOT(about()));

	 QAction *updates = new QAction(tr("&Check for updates..."), this);
	 updates->setMenuRole(QAction::ApplicationSpecificRole);
	 connect(updates, SIGNAL(triggered()), this, SLOT(checkForUpdates()));

	 helpMenu->addAction(updates);
}

void MainWindow::createMacDockMenu()
{
#ifndef Q_OS_MAC
	return;
#else
	QMenu *dockMenu = new QMenu(this);

	dockMenu->addAction(tr("New File"), this, SLOT(newFile()));

	qt_mac_set_dock_menu(dockMenu);
#endif
}

void MainWindow::showLicenceDialog()
{
	checkLicence(true);
}

void MainWindow::checkLicence(bool forceDialog)
{
	Licence l;

	if (!l.isValid() || l.hasExpired() || forceDialog)
	{
		LicenceCheckDialog dlg(this, &l);
		dlg.exec();

		//	If the licence is invalid after running the dialog, exit.
		l = Licence();
		if (!l.isValid() || l.hasExpired())
			close();
	}

	updateTrialToolbar();
	// If we're on a trial, add a timer to update the toolbar hourly
	if(!l.getExpiry().isNull())
	{
		mTrialTimer = new QTimer(this);
		connect(mTrialTimer, SIGNAL(timeout()), this, SLOT(updateTrialToolbar()));
		mTrialTimer->start(60*60*1000);
	}
}

void MainWindow::updateTrialToolbar()
{
	Licence l = Licence();

	//	Display a "time left on the trial" panel in the toolbar if appropriate
	if (!l.getExpiry().isNull())
	{
		mTrialRemainingButton->setText(tr("Trial: %1 days left").arg(l.getDaysLeft()));
		mTrialRemainingBar->show();
	}
	else
		mTrialRemainingBar->hide();
}

void MainWindow::showErrorMessage(QString error)
{
	QMessageBox::critical(this, "Error", error);
}

void MainWindow::showStatusMessage(QString message)
{
	mStatusLine->setText(message);
}

void MainWindow::undo()
{
	Editor* current = gWindowManager->currentEditor();
	if(current)
		current->undo();
}

void MainWindow::redo()
{
	Editor* current = gWindowManager->currentEditor();
	if(current)
		current->redo();
}

void MainWindow::cut()
{
	Editor* current = gWindowManager->currentEditor();
	if(current)
		current->cut();
}

void MainWindow::copy()
{
	Editor* current = gWindowManager->currentEditor();
	if(current)
		current->copy();
}

void MainWindow::paste()
{
	Editor* current = gWindowManager->currentEditor();
	if(current)
		current->paste();
}

void MainWindow::selectAll()
{
	Editor* current = gWindowManager->currentEditor();
	if(current)
		current->selectAll();
}

void MainWindow::showGotoLine()
{
	GotoLineDialog dlg(this);
	if(dlg.exec())
	{
		Editor* current = gWindowManager->currentEditor();
		if(current)
			current->gotoLine(dlg.lineNumber());
	}
}

void MainWindow::showAdvancedSearch()
{
	AdvancedSearchDialog dlg(this);

	connect(&dlg, SIGNAL(find(QString,bool,bool,bool)), gWindowManager, SLOT(find(QString,bool,bool,bool)));
	connect(&dlg, SIGNAL(globalFind(QString,QString,bool,bool,bool)), gWindowManager, SLOT(globalFind(QString,QString,bool,bool,bool)));

	connect(&dlg, SIGNAL(replace(QString,QString,bool,bool,bool)), gWindowManager, SLOT(replace(QString,QString,bool,bool,bool)));
	connect(&dlg, SIGNAL(globalReplace(QString,QString,QString,bool,bool,bool)), gWindowManager, SLOT(globalReplace(QString,QString,QString,bool,bool,bool)));

	dlg.exec();
}


void MainWindow::closeEvent(QCloseEvent* event)
{
	nextStartupPrompt();
	Tools::saveCurrentFiles();

	if (!gOpenFileManager.closeAllFiles())
	{
		event->ignore();
		return;
	}

	//	Save the geometry and toolbar state of this window on the way out
	QSettings settings;
	settings.setValue("mainwindow/geometry", saveGeometry());
	settings.setValue("mainwindow/state", saveState());
	QMainWindow::closeEvent(event);
}

void MainWindow::nextStartupPrompt()
{
	if(!Options::ShutdownPrompt)
		return;

	if(gWindowManager->getEditors()->length() == 0)
		return;

	ShutdownPrompt dlg(this);
	dlg.exec();
}

void MainWindow::syntaxMenuOptionClicked()
{
	QObject* eventSource = QObject::sender();
	QAction* action = static_cast<QAction*>(eventSource);
	QString syntaxName = action->data().toString();

	Editor* currentEditor = getCurrentEditor();
	if (!currentEditor) return;
	currentEditor->getFile()->setSyntax(syntaxName);
}

Editor* MainWindow::getCurrentEditor()
{
	return gWindowManager->currentEditor();
}

void MainWindow::currentEditorChanged()
{
	updateSyntaxSelection();
}

void MainWindow::updateSyntaxSelection()
{
	if (mCurrentSyntaxMenuItem != NULL)
	{
		mCurrentSyntaxMenuItem->setChecked(false);
		mCurrentSyntaxMenuItem = NULL;
	}

	Editor* editor = getCurrentEditor();
	if (editor)
	{
		mSyntaxMenu->setEnabled(true);
		BaseFile* file = editor->getFile();
		QString syntaxName = file->getSyntax();
		mCurrentSyntaxMenuItem = mSyntaxMenuEntries.value(syntaxName, NULL);
		if (mCurrentSyntaxMenuItem != NULL)
			mCurrentSyntaxMenuItem->setChecked(true);
	}
	else
		mSyntaxMenu->setEnabled(false);
}

void MainWindow::updateRecentFilesMenu()
{
	mRecentFilesMenu->clear();

	for(int ii = 0; ii < mRecentFiles.length(); ii++)
	{
		QAction* action = mRecentFilesMenu->addAction(mRecentFiles[ii].getDisplayPath(), this, SLOT(openSingleFile()));
		action->setData(ii);
	}
}

void MainWindow::addRecentFile(Location loc)
{
	if(loc.isNull())
		return;

	for(int ii = 0; ii < mRecentFiles.length(); ii++)
	{
		if(loc.getDisplayPath() == mRecentFiles[ii].getDisplayPath())
		{
			mRecentFiles.removeAt(ii);
			ii--;
		}
	}
	mRecentFiles.push_front(loc);

	// Only keep the 10 most recent files
	for(int ii = 9; ii < mRecentFiles.length(); ii++)
		mRecentFiles.removeAt(ii);

	updateRecentFilesMenu();

	Tools::saveRecentFiles(mRecentFiles);
}

void MainWindow::checkForUpdates()
{
	gSiteManager->checkForUpdates(true);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
		event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;

	QString fileName = urls.first().toLocalFile();
	if (fileName.isEmpty())
		return;

	openSingleFile(Location(fileName));
}

void MainWindow::showHTMLPreview()
{
	HTMLPreview* htmlPreview = new HTMLPreview(this);
	QDockWidget* htmlWrapper = new QDockWidget("HTML Preview", 0);

	connect(htmlWrapper, SIGNAL(visibilityChanged(bool)), this, SLOT(closeHTMLPreview(bool)));

	htmlWrapper->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
	htmlWrapper->setWidget(htmlPreview);
	addDockWidget(Qt::BottomDockWidgetArea, htmlWrapper, Qt::Horizontal);

	htmlWrapper->show();

	htmlWrapper->setObjectName(tr("HTML Preview"));
}

void MainWindow::closeHTMLPreview(bool visible)
{
	if(visible)
		return;

	QObject* eventSource = QObject::sender();
	QDockWidget* htmlWrapper = static_cast<QDockWidget*>(eventSource);
	//HTMLPreview* htmlPreview = (HTMLPreview*)htmlWrapper->widget();

	//delete htmlPreview;
	htmlWrapper->deleteLater();
}

void MainWindow::resetZoom()
{
	Options::EditorFontZoom = 100;

	gDispatcher->emitOptionsChanged();
}

void MainWindow::zoomIn()
{
	Options::EditorFontZoom += 10;

	gDispatcher->emitOptionsChanged();
}

void MainWindow::zoomOut()
{
	Options::EditorFontZoom -= 10;
	if(Options::EditorFontZoom < 10)
		Options::EditorFontZoom = 10;

	gDispatcher->emitOptionsChanged();
}

void MainWindow::print()
{
	Editor* current = getCurrentEditor();
	if(!current)
		return;

	QPrinter printer;

	QPrintDialog *dialog = new QPrintDialog(&printer, this);
	dialog->setWindowTitle(tr("Print Document"));

	if (dialog->exec() != QDialog::Accepted)
		return;

	current->print(&printer);
}

void MainWindow::toggleFullScreen()
{
	if(isFullScreen())
	{
		if(mWasMaximized)
			showMaximized();
		else
			showNormal();
	}
	else
	{
		if(isMaximized())
			mWasMaximized = true;
		else
			mWasMaximized = false;
		showFullScreen();
	}
}

//	Override for QMainWindow::createPopupMenu. Removes menu entries for things I don't want shown.
QMenu* MainWindow::createPopupMenu()
{
	QMenu* menu = new QMenu(this);

	foreach (QDockWidget* dockWidget, mMenuControlledDockWidgets)
		menu->addAction(dockWidget->toggleViewAction());

	menu->addSeparator();

	foreach (QToolBar* toolbar, mMenuControlledToolBar)
		menu->addAction(toolbar->toggleViewAction());

	return menu;
}

void MainWindow::switchToQuickList()
{
	mFileList->setVisible(true);
	mQuickListMenuItem->setChecked(true);

	mTabbedFileList->setVisible(false);
	mTabbedListMenuItem->setChecked(false);

	Options::FileListType = Options::QuickList;
}

void MainWindow::switchtoTabbedList()
{
	mTabbedFileList->setVisible(true);
	mTabbedListMenuItem->setChecked(true);

	mFileList->setVisible(false);
	mQuickListMenuItem->setChecked(false);

	Options::FileListType = Options::TabbedList;
}

void MainWindow::openFileListChanged()
{
	//	Enable / disable all actions that are dependant on open files
	bool filesOpen = (gOpenFileManager.getFileCount() > 0);
	foreach (QAction* action, mActionsRequiringFiles)
		action->setEnabled(filesOpen);
}

void MainWindow::viewSplittingChanged()
{
	//	Enable / disable all actions that are dependant on split views
	bool viewSplit = gWindowManager->isSplit();
	foreach (QAction* action, mActionsRequiringSplitViews)
		action->setEnabled(viewSplit);
}






