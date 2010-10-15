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
	QTextEdit* editor = new QTextEdit(this);
	setCentralWidget(editor);

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

	RemoteFile f = controller.openFile(filename.toUtf8());
}

MainWindow::~MainWindow()
{

}
