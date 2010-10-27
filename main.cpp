#include <QtGui/QApplication>
#include "mainwindow.h"
#include "sshconnection.h"
#include "location.h"
#include "tools.h"
#include <QString>
#include <QDebug>
#include <QMetaType>

int main(int argc, char *argv[])
{
	int result = 1;

	try
	{
		SshConnection::initializeLib();

		qRegisterMetaType<Location>("Location");
		qRegisterMetaType< QList<Location> >("QList<Location>");

		QCoreApplication::setOrganizationName("BananaMonkeyChainsaw");
		QCoreApplication::setApplicationName("RemoteEditor");

		Tools::loadServers();

		QApplication a(argc, argv);
		MainWindow w;
		w.show();

		result = a.exec();
	}
	catch (const char* err)
	{
		qDebug() << "FATAL ERROR: " << err;
	}

	LocationShared::cleanupIconProvider();
	return result;
}
