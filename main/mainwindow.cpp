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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	mUnsavedChangesDialog = NULL;
	mCurrentSyntaxMenuItem = NULL;

	setWindowTitle(tr("PonyEdit"));

	setAcceptDrops(true);

	mWindowManager = new WindowManager(this);
	mWindowManager->setMinimumWidth(200);
	setCentralWidget(mWindowManager);

	mFileList = new FileList();
	addDockWidget(Qt::LeftDockWidgetArea, mFileList, Qt::Vertical);
	mFileList->setObjectName("File List");

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
	connect(mWindowManager, SIGNAL(currentChanged()), this, SLOT(currentEditorChanged()), Qt::QueuedConnection);
	connect(gDispatcher, SIGNAL(syntaxChanged(BaseFile*)), this, SLOT(updateSyntaxSelection()));

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

	//QTimer::singleShot(0, this, SLOT(checkLicence()));
}

MainWindow::~MainWindow()
{
	delete mWindowManager;
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
	QToolBar* toolbar = new QToolBar("File Toolbar");
	toolbar->addAction(QIcon(":/icons/new.png"), "New", this, SLOT(newFile()));
	toolbar->addAction(QIcon(":/icons/open.png"), "Open", this, SLOT(openFile()));
	toolbar->addAction(QIcon(":/icons/save.png"), "Save", this, SLOT(saveFile()));

	this->addToolBar(toolbar);
	toolbar->setObjectName("File Toolbar");

	QToolBar* feedbackToolbar = new QToolBar("Feedback Toolbar");
	feedbackToolbar->setObjectName("Feedback Toolbar");

	QWidget* spacer = new QWidget();
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	feedbackToolbar->addWidget(spacer);

	QToolButton *feedbackButton = new QToolButton(feedbackToolbar);
	feedbackButton->setText(tr("Feedback") + " ");

	QMenu *feedbackMenu = new QMenu(toolbar);

	feedbackMenu->addAction(tr("PonyEdit Made Me Happy Because..."), gSiteManager, SLOT(feedbackHappy()));
	feedbackMenu->addAction(tr("PonyEdit Made Me Sad Because..."), gSiteManager, SLOT(feedbackSad()));

	feedbackButton->setMenu(feedbackMenu);

	feedbackButton->setPopupMode(QToolButton::InstantPopup);

	feedbackToolbar->addWidget(feedbackButton);

	this->addToolBar(feedbackToolbar);
}

void MainWindow::newFile()
{
	QString path = "";
	Location location(path);
	BaseFile* file = location.getFile();

	mWindowManager->displayFile(file);

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
			openSingleFile(&location);
	}
}

void MainWindow::openSingleFile(Location *loc)
{
	if(!loc)
	{
		QAction* action = static_cast<QAction*>(sender());
		if(action)
			loc = mRecentFiles[action->data().toInt()];
	}

	if (loc && !loc->isDirectory())
	{
		BaseFile* file = loc->getFile();
		if (file->isClosed())
			file->open();

		mWindowManager->displayFile(file);

		gDispatcher->emitSelectFile(file);

		addRecentFile(loc);

		connect(file, SIGNAL(openStatusChanged(int)), this, SLOT(updateTitle()));
		connect(file, SIGNAL(unsavedStatusChanged()), this, SLOT(updateTitle()));
	}
}

void MainWindow::saveFile()
{
	Editor* current = mWindowManager->currentEditor();
	if (current)
	{
		if(current->getFile()->getLocation().getProtocol() == Location::Unsaved)
			saveFileAs();
		else
			current->save();
	}
}

void MainWindow::saveFileAs()
{
	FileDialog dlg(this, true);
	if(dlg.exec())
	{
		Editor* current = mWindowManager->currentEditor();
		Location loc = dlg.getNewLocation();
		loc.getFile()->newFile(current->getFile()->getContent());

		loc.getFile()->open();

		openSingleFile(&loc);

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
	Editor* current = mWindowManager->currentEditor();
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

	BaseFile* current = mWindowManager->currentEditor()->getFile();

	foreach(BaseFile* file, openFiles)
		if(file != current)
			file->close();
}

void MainWindow::fileSelected(BaseFile* file)
{
	if (!file) return;

	updateTitle(file);

	mWindowManager->displayFile(file);
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
		if(editor == mWindowManager->currentEditor())
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

	if(file->getLocation().getProtocol() == Location::Local)
		setWindowFilePath(file->getLocation().getPath());
	else
	{
		setWindowFilePath("");
		QIcon icon = file->getLocation().getIcon();
		setWindowIcon(icon);
	}

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

	fileMenu->addAction(tr("&New File"), this, SLOT(newFile()),
						QKeySequence::New);

	fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()),
						QKeySequence::Open);

	mRecentFilesMenu = new QMenu(tr("&Recent Files"), fileMenu);
	fileMenu->addMenu(mRecentFilesMenu);

	fileMenu->addSeparator();

	fileMenu->addAction(tr("&Save"), this, SLOT(saveFile()),
						QKeySequence::Save);
#ifdef Q_OS_WIN
	fileMenu->addAction(tr("Save &As..."), this, SLOT(saveFileAs()));

	fileMenu->addAction(tr("Save A&ll"), this, SLOT(saveAllFiles()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
#else
	fileMenu->addAction(tr("Save &As..."), this, SLOT(saveFileAs()), QKeySequence::SaveAs);

	fileMenu->addAction(tr("Save A&ll"), this, SLOT(saveAllFiles()));
#endif

	fileMenu->addSeparator();

	fileMenu->addAction(tr("&Print..."), this, SLOT(print()),
						QKeySequence::Print);

	fileMenu->addSeparator();

	fileMenu->addAction(tr("&Close"), this, SLOT(closeFile()), QKeySequence(Qt::CTRL + Qt::Key_W));

	fileMenu->addAction(tr("Close All"), this, SLOT(closeAllFiles()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_W));

	fileMenu->addAction(tr("Close All Except Current"), this, SLOT(closeAllExceptCurrentFile()));

	fileMenu->addSeparator();

	fileMenu->addAction(tr("E&xit"), this, SLOT(close()),
						QKeySequence::Quit);
}

void MainWindow::createEditMenu()
{
	QMenu *editMenu = new QMenu(tr("&Edit"), this);
	menuBar()->addMenu(editMenu);

	editMenu->addAction(tr("&Undo"), this, SLOT(undo()), QKeySequence::Undo);
	editMenu->addAction(tr("&Redo"), this, SLOT(redo()), QKeySequence::Redo);

	editMenu->addSeparator();

	editMenu->addAction(tr("&Cut"), this, SLOT(cut()), QKeySequence::Cut);
	editMenu->addAction(tr("C&opy"), this, SLOT(copy()), QKeySequence::Copy);
	editMenu->addAction(tr("&Paste"), this, SLOT(paste()), QKeySequence::Paste);

	editMenu->addSeparator();

	editMenu->addAction(tr("Select &All"), this, SLOT(selectAll()), QKeySequence::SelectAll);

	editMenu->addSeparator();

	editMenu->addAction(tr("&Find/Replace"), mWindowManager, SLOT(showSearchBar()), QKeySequence::Find);
	editMenu->addAction("Find &Next", mWindowManager, SLOT(findNext()), QKeySequence::FindNext);
	editMenu->addAction("Find P&revious", mWindowManager, SLOT(findPrevious()), QKeySequence::FindPrevious);
	editMenu->addAction(tr("Advanced F&ind/Replace..."), this, SLOT(showAdvancedSearch()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
#ifdef Q_OS_MAC
	editMenu->addAction(tr("&Go To Line..."), this, SLOT(showGotoLine()), QKeySequence(Qt::CTRL + Qt::Key_L));
#else
	editMenu->addAction(tr("&Go To Line..."), this, SLOT(showGotoLine()), QKeySequence(Qt::CTRL + Qt::Key_G));
#endif
}

void MainWindow::createViewMenu()
{
	QMenu* viewMenu = new QMenu(tr("&View"), this);
	menuBar()->addMenu(viewMenu);

	viewMenu->addAction(tr("&Actual Size"), this, SLOT(resetZoom()), QKeySequence(Qt::CTRL + Qt::Key_0));
	viewMenu->addAction(tr("Zoom &In"), this, SLOT(zoomIn()), QKeySequence::ZoomIn);
	viewMenu->addAction(tr("Zoom &Out"), this, SLOT(zoomOut()), QKeySequence::ZoomOut);

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

	toolsMenu->addAction(tr("&HTML Preview..."), this, SLOT(showHTMLPreview()));

	toolsMenu->addAction(tr("&Options..."), this, SLOT(options()), QKeySequence::Preferences);
}

void MainWindow::createWindowMenu()
{
	QMenu *windowMenu = new QMenu(tr("&Window"), this);
	menuBar()->addMenu(windowMenu);

	windowMenu->addAction(tr("&Previous Window"), mWindowManager, SLOT(previousWindow()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab));
	windowMenu->addAction(tr("&Next Window"), mWindowManager, SLOT(nextWindow()), QKeySequence::NextChild);
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

void MainWindow::checkLicence()
{
	LicenceCheckDialog dlg;
	dlg.exec();
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
	Editor* current = mWindowManager->currentEditor();
	if(current)
		current->undo();
}

void MainWindow::redo()
{
	Editor* current = mWindowManager->currentEditor();
	if(current)
		current->redo();
}

void MainWindow::cut()
{
	Editor* current = mWindowManager->currentEditor();
	if(current)
		current->cut();
}

void MainWindow::copy()
{
	Editor* current = mWindowManager->currentEditor();
	if(current)
		current->copy();
}

void MainWindow::paste()
{
	Editor* current = mWindowManager->currentEditor();
	if(current)
		current->paste();
}

void MainWindow::selectAll()
{
	Editor* current = mWindowManager->currentEditor();
	if(current)
		current->selectAll();
}

void MainWindow::showGotoLine()
{
	GotoLineDialog dlg(this);
	if(dlg.exec())
	{
		Editor* current = mWindowManager->currentEditor();
		if(current)
			current->gotoLine(dlg.lineNumber());
	}
}

void MainWindow::showAdvancedSearch()
{
	AdvancedSearchDialog dlg(this);

	connect(&dlg, SIGNAL(find(QString,bool,bool,bool)), mWindowManager, SLOT(find(QString,bool,bool,bool)));
	connect(&dlg, SIGNAL(globalFind(QString,QString,bool,bool,bool)), mWindowManager, SLOT(globalFind(QString,QString,bool,bool,bool)));

	connect(&dlg, SIGNAL(replace(QString,QString,bool,bool,bool)), mWindowManager, SLOT(replace(QString,QString,bool,bool,bool)));
	connect(&dlg, SIGNAL(globalReplace(QString,QString,QString,bool,bool,bool)), mWindowManager, SLOT(globalReplace(QString,QString,QString,bool,bool,bool)));

	dlg.exec();
}


void MainWindow::closeEvent(QCloseEvent* event)
{
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
	return mWindowManager->currentEditor();
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
		QAction* action = mRecentFilesMenu->addAction(mRecentFiles[ii]->getDisplayPath(), this, SLOT(openSingleFile()));
		action->setData(ii);
	}
}

void MainWindow::addRecentFile(Location *loc)
{
	if(loc->getPath().isNull())
		return;

	Location* newLoc = new Location(*loc);

	for(int ii = 0; ii < mRecentFiles.length(); ii++)
	{
		if(newLoc->getDisplayPath() == mRecentFiles[ii]->getDisplayPath())
		{
			mRecentFiles.removeAt(ii);
			ii--;
		}
	}
	mRecentFiles.push_front(newLoc);

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

	Location loc(fileName);
	openSingleFile(&loc);
}

void MainWindow::showHTMLPreview()
{
	HTMLPreview *htmlPreview = new HTMLPreview(this);
	QDockWidget *htmlWrapper = new QDockWidget("HTML Preview", 0);

	htmlWrapper->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
	htmlWrapper->setWidget(htmlPreview);
	addDockWidget(Qt::BottomDockWidgetArea, htmlWrapper, Qt::Horizontal);

	htmlWrapper->show();

	htmlWrapper->setObjectName(tr("HTML Preview"));
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
