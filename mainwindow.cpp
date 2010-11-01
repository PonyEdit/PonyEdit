#include "mainwindow.h"

#include <QDebug>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QRegExp>
#include <QTime>
#include <QCryptographicHash>
#include <QPushButton>
#include <QToolBar>
#include "filedialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	mEditorStack = new QStackedWidget(this);
	setCentralWidget(mEditorStack);

	createToolbar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createToolbar()
{
	QToolBar* toolbar = new QToolBar("File");
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
				Editor* newEditor = new Editor(location);
				mEditorStack->addWidget(newEditor);
				mEditorStack->setCurrentWidget(newEditor);
			}
		}
	}
}

void MainWindow::saveFile()
{
}

