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
	ServerConfigDlg dlg(this);
	dlg.exec();
	QString hostname = dlg.getHostname();
	QString login = dlg.getLogin();
	QString password = dlg.getPassword();
	QString filename = dlg.getFilename();

	SshConnection* c = new SshConnection();
	c->connect(hostname.toUtf8(), 22);
	c->authenticatePassword(login.toUtf8(), password.toUtf8());

	mController = new SshRemoteController();
	mController->attach(c);

	QByteArray fileContent = mController->openFile(filename.toUtf8());

	mController->splitThread();

	mEditor = new QTextEdit(this);
	mEditor->setAcceptRichText(false);
	mEditor->setFont(QFont("courier new", 12));
	setCentralWidget(mEditor);

	mCurrentDocument = new QTextDocument(QString(fileContent));
	mEditor->setDocument(mCurrentDocument);
	mCurrentDocument->setDefaultFont(QFont("courier new", 12));

	createToolbar();

	connect(mCurrentDocument, SIGNAL(contentsChange(int,int,int)), this, SLOT(docChanged(int,int,int)));
}

MainWindow::~MainWindow()
{
}

void MainWindow::createToolbar()
{
	QToolBar* toolbar = new QToolBar("File");
	toolbar->addAction(QIcon("icons/new.png"), "New", this, SLOT(newFile()));
	toolbar->addAction(QIcon("icons/open.png"), "Open", this, SLOT(openFile()));
	toolbar->addAction(QIcon("icons/save.png"), "Save", this, SLOT(saveFile()));
	this->addToolBar(toolbar);
}

void MainWindow::docChanged(int position, int charsRemoved, int charsAdded)
{
	Push p;
	p.save = 0;
	p.position = position;
	p.remove = charsRemoved;

	p.add = "";
	for (int i = 0; i < charsAdded; i++)
		p.add += mCurrentDocument->characterAt(i + position);

	mController->push(p);
}

void MainWindow::newFile(){}
void MainWindow::openFile()
{
	FileDialog dlg(this);
	dlg.exec();
}

void MainWindow::saveFile()
{
	Push p;
	p.save = 1;
	mController->push(p);
}
