#include <QtGui/QApplication>
#include <QString>
#include <QDebug>
#include <QMetaType>
#include <QFont>

#include "main/globaldispatcher.h"
#include "ssh/rawsshconnection.h"
#include "file/location.h"
#include "main/tools.h"
#include "main/mainwindow.h"
#include "website/sitemanager.h"
#include "website/updatemanager.h"
#include "syntax/syntaxrule.h"
#include "syntax/syntaxdefmanager.h"
#include "main/stringtrie.h"
#include "options/options.h"
#include "ponyedit.h"

GlobalDispatcher* gDispatcher = NULL;
SiteManager* gSiteManager = NULL;
SyntaxDefManager* gSyntaxDefManager = NULL;
MainWindow* gMainWindow = NULL;

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
		QCoreApplication::setApplicationVersion("0.9-prealpha10");
		gDispatcher = new GlobalDispatcher();

		RawSshConnection::initializeLib();

		Tools::loadServers();
		Location::loadFavorites();
		Tools::initialize();

		PonyEdit a(argc, argv);

		if(a.isRunning())
		{
			if(argc > 1)
			{
				for(int ii = 1; ii < argc; ii++)
				{
					QString name(argv[ii]);
					if(name.trimmed().isNull())
						continue;

					a.sendMessage(name);
				}
			}

			return 0;
		}

		Options::load();

		gSyntaxDefManager = new SyntaxDefManager();
		gSiteManager = new SiteManager();
		updateManager = new UpdateManager();

		gMainWindow = new MainWindow();
		gMainWindow->show();

		Tools::loadStartupFiles();

		if(argc > 1)
		{
			for(int ii = 1; ii < argc; ii++)
			{
				QString name(argv[ii]);
				if(name.trimmed().isNull())
					continue;

				Location *loc = new Location(name);
				gMainWindow->openSingleFile(loc);
				delete loc;
			}
		}

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
