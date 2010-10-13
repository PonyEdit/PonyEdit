#include <QtGui/QApplication>
#include "mainwindow.h"
#include "sshconnection.h"
#include <QString>
#include <QDebug>

int main(int argc, char *argv[])
{
	int result = 1;

	try
	{
		SshConnection::initializeLib();

		QApplication a(argc, argv);
		MainWindow w;
		w.show();

		result = a.exec();
	}
	catch (const char* err)
	{
		qDebug() << "FATAL ERROR: " << err;
	}

	return result;
}
