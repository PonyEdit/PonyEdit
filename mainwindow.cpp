#include "mainwindow.h"

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
#include "filedialog.h"
#include "filelist.h"
#include "editor.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	mEditorStack = new QStackedWidget(this);
	mEditorStack->setMinimumWidth(200);
	setCentralWidget(mEditorStack);

	mFileList = new FileList();
	addDockWidget(Qt::LeftDockWidgetArea, mFileList, Qt::Vertical);
	connect(mFileList, SIGNAL(fileSelected(BaseFile*)), this, SLOT(fileSelected(BaseFile*)));

	createToolbar();

	createFileMenu();
	createToolsMenu();
	createHelpMenu();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createToolbar()
{
	QToolBar* toolbar = new QToolBar("File Toolbar");
	toolbar->addAction(QIcon(":/icons/new.png"), "New", this, SLOT(newFile()));
	toolbar->addAction(QIcon(":/icons/open.png"), "Open", this, SLOT(openFile()));
	toolbar->addAction(QIcon(":/icons/save.png"), "Save", this, SLOT(saveFile()));
	this->addToolBar(toolbar);
}

void MainWindow::newFile(){}
void MainWindow::openFile()
{
	FileDialog dlg(this);
	if (dlg.exec())
	{
		QList<Location> locations = dlg.getSelectedLocations();
		foreach (Location location, locations)
		{
			if (!location.isDirectory())
			{
				BaseFile* file = location.getFile();
				if (file->isUnopened())
					file->open();

				Editor* newEditor = new Editor(file);
				mEditorStack->addWidget(newEditor);
				mEditorStack->setCurrentWidget(newEditor);
				mEditors.append(newEditor);
			}
		}
	}
}

void MainWindow::saveFile()
{
	Editor* current = (Editor*)mEditorStack->currentWidget();
	current->save();
}

void MainWindow::fileSelected(BaseFile* file)
{
	const QList<Editor*>& editors = file->getAttachedEditors();
	if (editors.length() > 0)
		mEditorStack->setCurrentWidget(editors[0]);
}

void MainWindow::options()
{
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Remoted"),
					 tr("<p>It's awesome.</p><p>The End.</p>"));
}

void MainWindow::createFileMenu()
{
	QMenu *fileMenu = new QMenu(tr("&File"), this);
	menuBar()->addMenu(fileMenu);

	fileMenu->addAction(tr("&New"), this, SLOT(newFile()),
						QKeySequence::New);

	fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()),
						QKeySequence::Open);

	fileMenu->addAction(tr("&Save"), this, SLOT(saveFile()),
						QKeySequence::Save);

	fileMenu->addAction(tr("E&xit"), QCoreApplication::instance(), SLOT(quit()),
						QKeySequence::Quit);
}

void MainWindow::createToolsMenu()
{
	QMenu *toolsMenu = new QMenu(tr("&Tools"), this);
	menuBar()->addMenu(toolsMenu);

	toolsMenu->addAction(tr("&Options..."), this, SLOT(options()));
}

void MainWindow::createHelpMenu()
{
	 QMenu *helpMenu = new QMenu(tr("&Help"), this);
	 menuBar()->addMenu(helpMenu);

	 helpMenu->addAction(tr("&About"), this, SLOT(about()));
}
