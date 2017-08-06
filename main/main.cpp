#ifndef TESTPONY

HIDE_COMPILE_WARNINGS

#include <QtWidgets/QApplication>
#include <QString>
#include <QDebug>
#include <QMetaType>
#include <QFont>
#include <QNetworkProxyFactory>
#include <QStandardPaths>
#include <QDir>

UNHIDE_COMPILE_WARNINGS

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
        QDir datadir(QStandardPaths::writableLocation( QStandardPaths::DataLocation ) );
		if( !datadir.exists() )
            datadir.mkpath( QStandardPaths::writableLocation( QStandardPaths::DataLocation ) );

        fileDestination = QsLogging::DestinationPtr( QsLogging::DestinationFactory::MakeFileDestination( QStandardPaths::writableLocation( QStandardPaths::DataLocation ) + "/ponyedit.log" ) );
		debugDestination = QsLogging::DestinationPtr( QsLogging::DestinationFactory::MakeDebugOutputDestination() );
		logger.addDestination( debugDestination );
		logger.addDestination( fileDestination );

		QLOG_INFO() << "PonyEdit version" << PRETTY_VERSION << "started";

		PonyEdit a(argc, argv);

		const QStringList& positionalArguments = a.getPositionalArguments();
		if ( a.isRunning() ) {
			//	App is already running; just send it messages, to open given files (if any)
			foreach ( QString arg, positionalArguments )
				a.sendMessage( arg );
		} else {
			//	App not running; open given filenames here.
			foreach ( QString arg, positionalArguments )
				gMainWindow->openSingleFile( Location( arg ) );
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
