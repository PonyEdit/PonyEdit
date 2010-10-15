#include "mainwindow.h"

#include <QDebug>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QRegExp>
#include <QTime>
#include <QCryptographicHash>

#include "sshconnection.h"
#include "sshremotecontroller.h"
#include "serverconfigdlg.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	ServerConfigDlg dlg(this);
	dlg.exec();
	QString hostname = dlg.getHostname();
	QString login = dlg.getLogin();
	QString password = dlg.getPassword();
	QString filename = dlg.getFilename();

	SshConnection c;
	c.connect(hostname.toUtf8(), 22);
	c.authenticatePassword(login.toUtf8(), password.toUtf8());

	SshRemoteController controller;
	controller.attach(&c);

	QByteArray fileContent = controller.openFile(filename.toUtf8());

	mEditor = new QTextEdit(this);
	setCentralWidget(mEditor);

	mCurrentDocument = new QTextDocument(QString(fileContent));
	mEditor->setDocument(mCurrentDocument);

	connect(mCurrentDocument, SIGNAL(contentsChange(int,int,int)), this, SLOT(docChanged(int,int,int)));
}

MainWindow::~MainWindow()
{

}

void MainWindow::docChanged(int position, int charsRemoved, int charsAdded)
{
	qDebug() << position << charsRemoved << charsAdded;
}
