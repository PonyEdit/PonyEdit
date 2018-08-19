#ifndef TESTPONY

#include <QDebug>
#include <QDir>
#include <QFont>
#include <QMetaType>
#include <QNetworkProxyFactory>
#include <QStandardPaths>
#include <QString>
#include <QtWidgets/QApplication>

#include "global.h"
#include "ponyedit.h"
#include "QsLogDest.h"
#include "website/sitemanager.h"
#include "website/updatemanager.h"

int main( int argc, char *argv[] ) {
	int result = 1;

	UpdateManager *updateManager = nullptr;

	QsLogging::Logger &logger = QsLogging::Logger::instance();
	logger.setLoggingLevel( QsLogging::TraceLevel );

	try {
		QCoreApplication::setOrganizationName( "Pentalon" );
		QCoreApplication::setApplicationName( "PonyEdit" );
		QCoreApplication::setApplicationVersion( PRETTY_VERSION );

		// Create our data directory if it doesn't already exist
		QString datadirPath = QStandardPaths::writableLocation( QStandardPaths::DataLocation );
		QDir datadir( datadirPath );
		if ( ! datadir.exists() ) {
			datadir.mkpath( QStandardPaths::writableLocation( QStandardPaths::DataLocation ) );
		}

		QsLogging::DestinationPtrU fileDestination( QsLogging::DestinationFactory::MakeFileDestination( datadirPath +
		                                                                                                "/ponyedit.log" ) );
		QsLogging::DestinationPtrU debugDestination( QsLogging::DestinationFactory::MakeDebugOutputDestination() );

		logger.addDestination( std::move( debugDestination ) );
		logger.addDestination( std::move( fileDestination ) );

		QLOG_INFO() << "PonyEdit version" << PRETTY_VERSION << "started";

		PonyEdit a( argc, argv );

		const QStringList &positionalArguments = a.getPositionalArguments();
		if ( a.isRunning() ) {
			// App is already running; just send it messages, to open given files (if any)
			foreach ( QString arg, positionalArguments ) {
				a.sendMessage( arg );
			}
		} else {
			// App not running; open given filenames here.
			foreach ( QString arg, positionalArguments ) {
				gMainWindow->openSingleFile( Location( arg ) );
			}
		}

		QNetworkProxyFactory::setUseSystemConfiguration( true );

		updateManager = new UpdateManager();

		QTimer::singleShot( 1000, gSiteManager, SLOT(checkForUpdates()) );

		result = PonyEdit::exec();
	} catch ( QString &err ) {
		QLOG_ERROR() << "FATAL ERROR: " << err;
	}

	QLOG_INFO() << "PonyEdit exited";

	delete updateManager;
	return result;
}

#endif
