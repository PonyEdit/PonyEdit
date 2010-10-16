#include "mainwindow.h"

#include <QDebug>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QRegExp>
#include <QTime>
#include <QCryptographicHash>
#include <QPushButton>
#include <QToolBar>

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

	QToolBar* toolbar = new QToolBar();
	toolbar->addAction("Save", this, SLOT(save()));
	this->addToolBar(toolbar);

	connect(mCurrentDocument, SIGNAL(contentsChange(int,int,int)), this, SLOT(docChanged(int,int,int)));
}

MainWindow::~MainWindow()
{

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

void MainWindow::save()
{
	Push p;
	p.save = 1;
	mController->push(p);
}
