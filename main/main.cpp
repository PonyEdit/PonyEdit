#include <QtGui/QApplication>
#include <QString>
#include <QDebug>
#include <QMetaType>
#include <QFont>

#include "main/globaldispatcher.h"
#include "ssh/sshconnection.h"
#include "file/location.h"
#include "main/tools.h"
#include "main/mainwindow.h"
#include "website/sitemanager.h"
#include "syntax/syntaxrule.h"

GlobalDispatcher* gDispatcher = NULL;
SiteManager* gSiteManager = NULL;

int main(int argc, char *argv[])
{
	int result = 1;

	try
	{
		SshConnection::initializeLib();

#ifndef Q_OS_LINUX
		QStringList substitutions;
		substitutions.append("consolas");
		substitutions.append("courier new");
		substitutions.append("helvetica");
		QFont::insertSubstitutions("inconsolata", substitutions);
#endif

		qRegisterMetaType<Location>("Location");
		qRegisterMetaType< QList<Location> >("QList<Location>");

		QCoreApplication::setOrganizationName("BananaMonkeyChainsaw");
		QCoreApplication::setApplicationName("RemoteEditor");
		QCoreApplication::setApplicationVersion("0.9-prealpha");
		gDispatcher = new GlobalDispatcher();

		Tools::loadServers();
		Tools::initialize();

		QApplication a(argc, argv);
		MainWindow w;
		w.show();

		gSiteManager = new SiteManager();
		gSiteManager->checkForUpdates();

		result = a.exec();
	}
	catch (QString err)
	{
		qDebug() << "FATAL ERROR: " << err;
	}

	delete gDispatcher;
	delete gSiteManager;
	LocationShared::cleanupIconProvider();
	SyntaxRule::cleanup();
	return result;
}
