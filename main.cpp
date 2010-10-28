#include <QtGui/QApplication>
#include "mainwindow.h"
#include "sshconnection.h"
#include "location.h"
#include "tools.h"
#include "globaldispatcher.h"
#include <QString>
#include <QDebug>
#include <QMetaType>

GlobalDispatcher* gDispatcher = NULL;

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
		gDispatcher = new GlobalDispatcher();

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

	delete gDispatcher;
	LocationShared::cleanupIconProvider();
	return result;
}
