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
#include "website/updatemanager.h"
#include "syntax/syntaxrule.h"
#include "syntax/syntaxdefmanager.h"
#include "main/stringtrie.h"
#include "options/options.h"

GlobalDispatcher* gDispatcher = NULL;
SiteManager* gSiteManager = NULL;
SyntaxDefManager* gSyntaxDefManager = NULL;

int main(int argc, char *argv[])
{
	int result = 1;

	UpdateManager* updateManager = NULL;

	try
	{
#ifndef Q_OS_LINUX
		QStringList substitutions;
		substitutions.append("consolas");
		substitutions.append("courier new");
		substitutions.append("helvetica");
		QFont::insertSubstitutions("inconsolata", substitutions);
#endif

		qRegisterMetaType<Location>("Location");
		qRegisterMetaType< QList<Location> >("QList<Location>");

		QCoreApplication::setOrganizationName("Pentalon");
		QCoreApplication::setApplicationName("PonyEdit");
		QCoreApplication::setApplicationVersion("0.9-prealpha2");
		gDispatcher = new GlobalDispatcher();

		SshConnection::initializeLib();

		Options::load();
		Tools::loadServers();
		Location::loadFavorites();
		Tools::initialize();

		QApplication a(argc, argv);

		gSyntaxDefManager = new SyntaxDefManager();

		MainWindow w;
		w.show();

		gSiteManager = new SiteManager();

		updateManager = new UpdateManager();

		gSiteManager->checkForUpdates();

		result = a.exec();
	}
	catch (QString err)
	{
		qDebug() << "FATAL ERROR: " << err;
	}

	delete updateManager;
	delete gDispatcher;
	delete gSiteManager;
	Options::save();
	LocationShared::cleanupIconProvider();
	StringTrie::cleanup();
	SyntaxRule::cleanup();
	return result;
}
