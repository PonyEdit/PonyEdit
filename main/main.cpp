#ifndef TESTPONY

#include <QtWidgets/QApplication>
#include <QString>
#include <QDebug>
#include <QMetaType>
#include <QFont>
#include <QNetworkProxyFactory>
#include <QDesktopServices>
#include <QDir>

#include "global.h"
#include "QsLogDest.h"
#include "website/updatemanager.h"
#include "website/sitemanager.h"
#include "ponyedit.h"

int main(int argc, char *argv[])
{
	int result = 1;

	UpdateManager* updateManager = NULL;

	QsLogging::Logger& logger = QsLogging::Logger::instance();
	logger.setLoggingLevel( QsLogging::TraceLevel );
	QsLogging::DestinationPtr fileDestination, debugDestination;

	try
	{
		QCoreApplication::setOrganizationName("Pentalon");
		QCoreApplication::setApplicationName("PonyEdit");
		QCoreApplication::setApplicationVersion(PRETTY_VERSION);

		// Create our data directory if it doesn't already exist
		QDir datadir(QDesktopServices::storageLocation( QDesktopServices::DataLocation ) );
		if( !datadir.exists() )
			datadir.mkpath( QDesktopServices::storageLocation( QDesktopServices::DataLocation ) );

		fileDestination = QsLogging::DestinationPtr( QsLogging::DestinationFactory::MakeFileDestination( QDesktopServices::storageLocation( QDesktopServices::DataLocation ) + "/ponyedit.log" ) );
		debugDestination = QsLogging::DestinationPtr( QsLogging::DestinationFactory::MakeDebugOutputDestination() );
		logger.addDestination( debugDestination.get() );
		logger.addDestination( fileDestination.get() );

		QLOG_INFO() << "PonyEdit version" << PRETTY_VERSION << "started";

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

		if(argc > 1)
		{
			for(int ii = 1; ii < argc; ii++)
			{
				QString name(argv[ii]);
				if(name.trimmed().isNull())
					continue;

				gMainWindow->openSingleFile(Location(name));
			}
		}

		QNetworkProxyFactory::setUseSystemConfiguration(true);

		updateManager = new UpdateManager();

		QTimer::singleShot(1000, gSiteManager, SLOT(checkForUpdates()));

		result = a.exec();
	}
	catch (QString err)
	{
		QLOG_ERROR() << "FATAL ERROR: " << err;
	}

	QLOG_INFO() << "PonyEdit exited";

	delete updateManager;
	return result;
}

#endif
