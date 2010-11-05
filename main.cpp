#include <QtGui/QApplication>
#include <QString>
#include <QDebug>
#include <QMetaType>

#include "globaldispatcher.h"
#include "sshconnection.h"
#include "location.h"
#include "tools.h"
#include "mainwindow.h"

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
		Tools::initialize();

		QApplication a(argc, argv);
		MainWindow w;
		w.show();

		result = a.exec();
	}
	catch (QString err)
	{
		qDebug() << "FATAL ERROR: " << err;
	}

	delete gDispatcher;
	LocationShared::cleanupIconProvider();
	return result;
}
