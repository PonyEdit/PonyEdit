#include "main/mainwindow.h"

#include <QDebug>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QRegExp>
#include <QTime>
#include <QCryptographicHash>
#include <QPushButton>
#include <QToolBar>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QCoreApplication>
#include <QSettings>
#include <QErrorMessage>

#include "file/filedialog.h"
#include "file/filelist.h"
#include "editor/editor.h"
#include "options/optionsdialog.h"
#include "main/globaldispatcher.h"
#include "main/searchbar.h"
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
#include "openssl/rsa.h"
#include "openssl/sha.h"
#include "openssl/bio.h"
#include "openssl/x509.h"
#include "openssl/evp.h"
#include "openssl/err.h"
#include "openssl/pem.h"


char* publicKey =	"-----BEGIN PUBLIC KEY-----\n"
					"MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA7AuOwBjtW4J8xrt+PGKv\n"
					"bOkxDDxk/smi6W2S4g8WIMSNq5GvX4Cg5TgRzzspgkKipZimuF9iQvNEUempmAQy\n"
					"deXARby8PnVtF35mhZomt7X48v57Wgha2nFLpz5/7jabguKyc0n7ox1lRdTfxLWO\n"
					"lzBf4FLRLyDNCXPqCQCmFSV35NPEavVHvdjtX/eTnRF6b2yEdSWT3LEmtMnMuHJT\n"
					"wD5Y1B/UwEv4q3IPO6p4Ebe6VuvwsRuBq9AHS5Jqzi3y7DJwVurrfMx/1eVynSUu\n"
					"VEDHcKkDBhjFK7ciz3Rq0iBHStolrSrwqQT2slb5caxg572HrfBd7A9CBF7j7MKt\n"
					"UwIDAQAB\n"
					"-----END PUBLIC KEY-----\n";

bool verifyLicenseKey(const char* licenseKey)
{
	QByteArray decoded = QByteArray::fromBase64(licenseKey);
	int firstDivider = decoded.indexOf(':');
	int secondDivider = decoded.indexOf(':', firstDivider + 1);
	if (firstDivider < 0 || secondDivider < 0)
		return false;

	QString login = decoded.mid(0, firstDivider);
	QString expiry = decoded.mid(firstDivider + 1, secondDivider - firstDivider - 1);

	QByteArray signedData = decoded.mid(0, secondDivider);
	QByteArray signature = decoded.mid(secondDivider + 1);

	//	SHA1-hash the signed data for verification
	SHA_CTX sha_ctx = { 0 };
	unsigned char digest[SHA_DIGEST_LENGTH];
	if (SHA1_Init(&sha_ctx) != 1) { qDebug() << "Failed to init SHA1 libs"; return false; }
	if (SHA1_Update(&sha_ctx, signedData.constData(), signedData.length()) != 1) { qDebug() << "Failed to add data to SHA1 hash"; return false; }
	if (SHA1_Final(digest, &sha_ctx) != 1) { qDebug() << "Failed to finalize SHA1 hash"; return false; }

	BIO* b = BIO_new_mem_buf(publicKey, strlen(publicKey));
	EVP_PKEY* k = PEM_read_bio_PUBKEY(b, NULL, NULL, NULL);

	bool result = RSA_verify(NID_sha1, digest, sizeof(digest), (const unsigned char*)signature.constData(), signature.length(), EVP_PKEY_get1_RSA(k));

	if (result)
		qDebug() << "License for " << login << " verified. Expires " << expiry;
	else
		qDebug() << "License rejected :(";

	EVP_PKEY_free(k);
	BIO_free(b);

	return result;
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	mUnsavedChangesDialog = NULL;
	mCurrentSyntaxMenuItem = NULL;

	setWindowTitle(tr("PonyEdit"));

	setAcceptDrops(true);

	mEditorStack = new QStackedWidget(this);
	mEditorStack->setMinimumWidth(200);
	setCentralWidget(mEditorStack);

	mFileList = new FileList();
	addDockWidget(Qt::LeftDockWidgetArea, mFileList, Qt::Vertical);
	mFileList->setObjectName("File List");

	createSearchBar();

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

	connect(gDispatcher, SIGNAL(generalErrorMessage(QString)), this, SLOT(showErrorMessage(QString)), Qt::QueuedConnection);
	connect(gDispatcher, SIGNAL(generalStatusMessage(QString)), this, SLOT(showStatusMessage(QString)), Qt::QueuedConnection);
	connect(gDispatcher, SIGNAL(selectFile(BaseFile*)), this, SLOT(fileSelected(BaseFile*)));
	connect(&gOpenFileManager, SIGNAL(fileClosed(BaseFile*)), this, SLOT(fileClosed(BaseFile*)), Qt::DirectConnection);
	connect(mEditorStack, SIGNAL(currentChanged(int)), this, SLOT(currentEditorChanged()));
	connect(gDispatcher, SIGNAL(syntaxChanged(BaseFile*)), this, SLOT(updateSyntaxSelection()));

	mRecentFiles = Tools::loadRecentFiles();
	updateRecentFilesMenu();

	verifyLicenseKey("dGhpbmdhbG9uOjIwMTEtMDYtMTI6h8jRlxHjYu1kfSJI1clyPozKGOhMW4MpBLJMBztnp7JO0oGnX437wI1rACRziiL4css2o95FRriyGwlZQZlts1SS7q+1qsOa9oGblD9lT0dAk/Dz5A5HIhd530qSjCt1cZMTcoD01xNuBYYVWvZny8NYDWG7A/OjRGQb3Bt6N1QtKrW9vPe6GA9AEj3ZJZXEX4BOa0LOXcNFmGOhNdn7U8lmxjNFiaPT+4LcPsB1iEH/uamuhs/QlrQwFg2jOh1SFBUwkvjMgEcvKvN4WjsI+j6GlxdAgtGu/2UiW3dcyFEIaDRV9iW0SyvkKvKfLWfGFaFWFecgD1pYAKn7hiJdNQ==");

	restoreState();
}

MainWindow::~MainWindow()
{
}

void MainWindow::restoreState()
{
	QSettings settings;
	restoreGeometry(settings.value("mainwindow/geometry").toByteArray());
	QMainWindow::restoreState(settings.value("mainwindow/state").toByteArray());
}

void MainWindow::createSearchBar()
{
	mSearchBar = new SearchBar();
	mSearchBarWrapper = new QDockWidget("Search", 0, Qt::FramelessWindowHint);
	mSearchBarWrapper->setFeatures(QDockWidget::DockWidgetClosable);
	mSearchBarWrapper->setWidget(mSearchBar);
	addDockWidget(Qt::BottomDockWidgetArea, mSearchBarWrapper, Qt::Horizontal);
	mSearchBarWrapper->hide();
	mSearchBarWrapper->setTitleBarWidget(new QWidget(this));
	connect(mSearchBar, SIGNAL(closeRequested()), mSearchBarWrapper, SLOT(hide()));
	connect(mSearchBar, SIGNAL(find(QString,bool)), this, SLOT(find(QString,bool)));
	connect(mSearchBar, SIGNAL(replace(QString,QString,bool)), this, SLOT(replace(QString,QString,bool)));
	mSearchBarWrapper->setObjectName("Search Bar");
}

void MainWindow::createToolbar()
{
	QToolBar* toolbar = new QToolBar("File Toolbar");
	toolbar->addAction(QIcon(":/icons/new.png"), "New", this, SLOT(newFile()));
	toolbar->addAction(QIcon(":/icons/open.png"), "Open", this, SLOT(openFile()));
	toolbar->addAction(QIcon(":/icons/save.png"), "Save", this, SLOT(saveFile()));
	this->addToolBar(toolbar);
	toolbar->setObjectName("File Toolbar");
}

void MainWindow::newFile()
{
	QString path = "";
	Location location(path);
	BaseFile* file = location.getFile();

	Editor* newEditor = new Editor(file);
	mEditorStack->addWidget(newEditor);
	mEditorStack->setCurrentWidget(newEditor);
	mEditors.append(newEditor);

	gDispatcher->emitSelectFile(file);

	newEditor->setFocus();
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

		Editor* newEditor = new Editor(file);
		mEditorStack->addWidget(newEditor);
		mEditorStack->setCurrentWidget(newEditor);
		mEditors.append(newEditor);

		gDispatcher->emitSelectFile(file);

		newEditor->setFocus();

		addRecentFile(loc);

		connect(file, SIGNAL(openStatusChanged(int)), this, SLOT(updateTitle()));
		connect(file, SIGNAL(unsavedStatusChanged()), this, SLOT(updateTitle()));
		connect(file, SIGNAL(saveFailed(QString)), this, SLOT(saveFailed(QString)));
	}
}

void MainWindow::saveFile()
{
	Editor* current = (Editor*)mEditorStack->currentWidget();
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
		Editor* current = (Editor*)mEditorStack->currentWidget();
		Location loc = dlg.getNewLocation();
		loc.getFile()->newFile(current->getFile()->getContent());

		openSingleFile(&loc);

		saveFile();

		if(current->getFile()->getLocation().getProtocol() == Location::Unsaved)
			current->close();
	}
}

void MainWindow::closeFile()
{
	Editor* current = (Editor*)mEditorStack->currentWidget();
	if (current)
	{
		QList<BaseFile *> files;
		files.append(current->getFile());

		gOpenFileManager.closeFiles(files);
	}
}

void MainWindow::fileSelected(BaseFile* file)
{
	if (!file) return;

	updateTitle(file);

	const QList<Editor*>& editors = file->getAttachedEditors();
	if (editors.length() > 0)
		mEditorStack->setCurrentWidget(editors[0]);
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
		if(editor == (Editor*)mEditorStack->currentWidget())
			current = true;
	}

	if(current)
		updateTitle(file);
}

void MainWindow::updateTitle(BaseFile* file)
{
	QString title = "PonyEdit - ";
	title += file->getLocation().getLabel();
	if(file->hasUnsavedChanges())
		title += '*';

	setWindowTitle(title);
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

	fileMenu->addAction(tr("Save &As..."), this, SLOT(saveFileAs()),
						QKeySequence::SaveAs);

	fileMenu->addSeparator();

	fileMenu->addAction(tr("&Close File"), this, SLOT(closeFile()),
						QKeySequence::Close);

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

	editMenu->addAction(tr("&Find/Replace"), this, SLOT(showSearchBar()), QKeySequence::Find);
	editMenu->addAction("Find &Next", mSearchBar, SLOT(findNext()), QKeySequence::FindNext);
	editMenu->addAction("Find P&revious", mSearchBar, SLOT(findPrev()), QKeySequence::FindPrevious);
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

	windowMenu->addAction(tr("&Previous Window"), this, SLOT(previousWindow()), QKeySequence::PreviousChild);
	windowMenu->addAction(tr("&Next Window"), this, SLOT(nextWindow()), QKeySequence::NextChild);
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

void MainWindow::showErrorMessage(QString error)
{
	QMessageBox::critical(this, "Error", error);
}

void MainWindow::showStatusMessage(QString message)
{
	mStatusLine->setText(message);
}

void MainWindow::undo(){}

void MainWindow::redo(){}

void MainWindow::cut(){}

void MainWindow::copy(){}

void MainWindow::paste(){}

void MainWindow::selectAll(){}

void MainWindow::showGotoLine()
{
	GotoLineDialog dlg(this);
	if(dlg.exec())
	{
		Editor* current = (Editor*)mEditorStack->currentWidget();
		if(current)
			current->gotoLine(dlg.lineNumber());
	}
}

void MainWindow::showSearchBar()
{
	mSearchBarWrapper->show();
	mSearchBar->takeFocus();
}

void MainWindow::showAdvancedSearch()
{
	AdvancedSearchDialog dlg(this);

	connect(&dlg, SIGNAL(find(QString,bool,bool,bool)), this, SLOT(find(QString,bool,bool,bool)));
	connect(&dlg, SIGNAL(globalFind(QString,QString,bool,bool,bool)), this, SLOT(globalFind(QString,QString,bool,bool,bool)));

	connect(&dlg, SIGNAL(replace(QString,QString,bool,bool,bool)), this, SLOT(replace(QString,QString,bool,bool,bool)));
	connect(&dlg, SIGNAL(globalReplace(QString,QString,QString,bool,bool,bool)), this, SLOT(globalReplace(QString,QString,QString,bool,bool,bool)));

	dlg.exec();
}

void MainWindow::find(const QString &text, bool backwards)
{
	Editor* current = (Editor*)mEditorStack->currentWidget();
	int found;
	if (current)
		found = find((Editor*)current, text, backwards, false, false);
}

void MainWindow::find(const QString &text, bool backwards, bool caseSensitive, bool useRegexp)
{
	Editor* current = (Editor*)mEditorStack->currentWidget();
	int found;
	if (current)
		found = find(current, text, backwards, caseSensitive, useRegexp);
}

void MainWindow::globalFind(const QString &text, const QString &filePattern, bool backwards, bool caseSensitive, bool useRegexp)
{
	int found = 0;

	Editor* current;
	BaseFile* file;
	Location loc;

#ifdef Q_OS_WIN
	QRegExp regexp(filePattern, Qt::CaseInsensitive, QRegExp::Wildcard);
#else
	QRegExp regexp(filePattern, Qt::CaseInsensitive, QRegExp::WildcardUnix);
#endif

	int filesSearched = 0;
	for(int ii = mEditorStack->currentIndex(); filesSearched < mEditorStack->count(); (backwards)?(ii--):(ii++))
	{
		current = (Editor*)mEditorStack->widget(ii);
		if(current)
		{
			file = current->getFile();
			loc = file->getLocation();

			if(regexp.exactMatch(loc.getDisplayPath()) || regexp.exactMatch(loc.getLabel()))
			{
				gDispatcher->emitSelectFile(file);
				current->setFocus();
				if(filesSearched > 0)
				{
					if(!backwards)
						current->gotoLine(1);
					else
						current->gotoEnd();
				}

				found += find(current, text, backwards, caseSensitive, useRegexp, false);
				if(found)
					break;
			}
		}
		filesSearched++;

		if(ii == 0 && backwards)
			ii = mEditorStack->count();
		else if(ii == mEditorStack->count() - 1 && !backwards)
			ii = -1;
	}
}

int MainWindow::find(Editor *editor, const QString &text, bool backwards, bool caseSensitive, bool useRegexp, bool loop)
{
	return editor->find(text, backwards, caseSensitive, useRegexp, loop);
}

void MainWindow::replace(const QString &findText, const QString &replaceText, bool all)
{
	Editor* current = (Editor*)mEditorStack->currentWidget();
	int replaced;
	if (current)
		replaced = replace(current, findText, replaceText, false, false, all);
}

void MainWindow::replace(const QString &findText, const QString &replaceText, bool caseSensitive, bool useRegexp, bool all)
{
	Editor* current = (Editor*)mEditorStack->currentWidget();
	int replaced;
	if (current)
		replaced = replace(current, findText, replaceText, caseSensitive, useRegexp, all);
}

void MainWindow::globalReplace(const QString &findText, const QString &replaceText, const QString &filePattern, bool caseSensitive, bool useRegexp, bool all)
{
	int replaced = 0;

	Editor* current;
	BaseFile* file;
	Location loc;

#ifdef Q_OS_WIN
	QRegExp regexp(filePattern, Qt::CaseInsensitive, QRegExp::Wildcard);
#else
	QRegExp regexp(filePattern, Qt::CaseInsensitive, QRegExp::WildcardUnix);
#endif

	int ii;
	if(all)
		ii = 0;
	else
		ii = mEditorStack->currentIndex();

	for(; ii < mEditorStack->count(); ii++)
	{
		current = (Editor*)mEditorStack->widget(ii);
		if(current)
		{
			file = current->getFile();
			loc = file->getLocation();

			if(regexp.exactMatch(loc.getDisplayPath()) || regexp.exactMatch(loc.getLabel()))
			{
				if(!all)
				{
					gDispatcher->emitSelectFile(file);
					current->setFocus();
				}

				replaced += replace(current, findText, replaceText, caseSensitive, useRegexp, all);
				if(replaced && !all)
					break;
			}
		}
	}
}

int MainWindow::replace(Editor *editor, const QString &findText, const QString &replaceText, bool caseSensitive, bool useRegexp, bool all)
{
	return editor->replace(findText, replaceText, caseSensitive, useRegexp, all);
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

void MainWindow::fileClosed(BaseFile* file)
{
	for (int i = 0; i < mEditors.length(); i++)
	{
		Editor* e = mEditors[i];
		if (e->getFile() == file)
		{
			mEditors.removeAt(i);
			i--;

			delete e;
		}
	}
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
	return static_cast<Editor*>(mEditorStack->currentWidget());
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

void MainWindow::nextWindow()
{
	Editor *next;

	if(mEditorStack->currentIndex() + 1 == mEditorStack->count())
		next = (Editor*)mEditorStack->widget(0);
	else
		next = (Editor*)mEditorStack->widget(mEditorStack->currentIndex() + 1);

	gDispatcher->emitSelectFile(next->getFile());
	next->setFocus();
}

void MainWindow::previousWindow()
{
	Editor *prev;

	if(mEditorStack->currentIndex() == 0)
		prev = (Editor*)mEditorStack->widget(mEditorStack->count() - 1);
	else
		prev = (Editor*)mEditorStack->widget(mEditorStack->currentIndex() - 1);

	gDispatcher->emitSelectFile(prev->getFile());
	prev->setFocus();
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

void MainWindow::saveFailed(const QString &error)
{
	QMessageBox dlg(this);

	dlg.setWindowTitle(tr("Unable to Save"));
	dlg.setText(tr("Unable to save this file."));
	dlg.setInformativeText(error);
	dlg.setIcon(QMessageBox::Critical);
	dlg.setStandardButtons(QMessageBox::Ok);

	dlg.exec();
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
