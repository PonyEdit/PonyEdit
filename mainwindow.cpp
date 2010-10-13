#include "mainwindow.h"

#include <QDebug>
#include <QTextEdit>
#include <QSyntaxHighlighter>
#include <QRegExp>
#include <QCryptographicHash>

#include "sshconnection.h"
#include "sshremotecontroller.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	QTextEdit* editor = new QTextEdit(this);
	setCentralWidget(editor);

	SshConnection c;
	c.connect("trouble.net.au", 22);
	c.authenticatePassword("thingalon", "kr4n5k1");

	SshRemoteController controller;
	controller.attach(&c);

}

MainWindow::~MainWindow()
{

}
