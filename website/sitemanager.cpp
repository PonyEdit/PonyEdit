#include <QCoreApplication>
#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QVariantMap>

#include "sitemanager.h"
#include "updatemanager.h"

#ifdef Q_OS_WIN
	#define DOWNLOAD_NAME "PonyEdit.exe"
#endif
#ifdef Q_OS_MAC
	#define DOWNLOAD_NAME "PonyEdit.dmg"
#endif
#ifdef Q_OS_LINUX
	#define DOWNLOAD_NAME "PonyEdit.rpm"
#endif

SiteManager::SiteManager() {
	mManager = new QNetworkAccessManager( this );
	connect( mManager, SIGNAL( finished( QNetworkReply * ) ), this, SLOT( handleReply( QNetworkReply * ) ) );
}

SiteManager::~SiteManager() {
	// Clean out any leftover replies
	foreach ( QNetworkReply *reply, mReplies.keys() ) {
		delete reply;
	}

	if ( mManager ) {
		delete mManager;
	}

	mManager = NULL;
}

void SiteManager::checkForUpdates( bool forceNotification ) {
	QUrl url( "https://api.github.com/repos/PonyEdit/PonyEdit/releases" );
	QNetworkReply *reply = mManager->get( QNetworkRequest( url ) );

	mReplies.insert( reply, forceNotification ? UpdateCheckForcedNotification : UpdateCheck );
}

void SiteManager::handleReply( QNetworkReply *reply ) {
	if ( ! mReplies.contains( reply ) ) {
		return;
	}
	Messages message = mReplies.value( reply );

	try {
		if ( reply->error() != QNetworkReply::NoError ) {
			throw( reply->errorString() );
		}

		QByteArray result = reply->readAll();

		QJsonParseError error;
		QVariant data = QJsonDocument::fromJson( result, &error ).toVariant();
		if ( error.error ) {
			throw( tr( "Failed to parse reply from website" ) );
		}

		switch ( message ) {
		case UpdateCheck:
		case UpdateCheckForcedNotification:
			handleUpdateCheckReply( data.toList(), message == UpdateCheckForcedNotification );
			break;
		}
	} catch ( QString &error ) {
		switch ( message ) {
		case UpdateCheck:
		case UpdateCheckForcedNotification:

			// Do nothing
			break;
		}
	}

	mReplies.remove( reply );
	reply->deleteLater();
}

void SiteManager::handleUpdateCheckReply( QList< QVariant > reply, bool forceNotification ) {
	int update_major = MAJOR_VERSION;
	int update_minor = MINOR_VERSION;
	int update_revision = REVISION;
	QString update_version;
	QString update_url;
	QStringList update_alerts;
	QStringList update_changes;

	QRegExp notNumeric( "[^0-9]+" );
	QRegExp newline( "[\r\n]+" );

	foreach ( QVariant rawEntry, reply ) {
		QVariantMap entry = rawEntry.toMap();

		// Split the version number...
		QString version = entry["tag_name"].toString();
		QStringList versionPieces = version.split( notNumeric );
		int major = versionPieces.empty() ? 0 : versionPieces.takeFirst().toInt();
		int minor = versionPieces.empty() ? 0 : versionPieces.takeFirst().toInt();
		int revision = versionPieces.empty() ? 0 : versionPieces.takeFirst().toInt();

		// Find a download URL...
		QString entry_url;
		foreach ( QVariant rawAsset, entry["assets"].toList() ) {
			QVariantMap asset = rawAsset.toMap();
			if ( asset["name"].toString() == DOWNLOAD_NAME ) {
				// This asset is for this OS :)
				entry_url = asset["url"].toString();
			}
		}
		if ( entry_url.isEmpty() ) {
			continue;
		}

		// If this is a newer release than currently installed
		if ( major > MAJOR_VERSION ||
		     ( major == MAJOR_VERSION && minor > MINOR_VERSION ) ||
		     ( major == MAJOR_VERSION && minor == MINOR_VERSION && revision > REVISION ) ) {

			QStringList releaseNotes = entry["body"].toString().split( newline );
			foreach ( QString note, releaseNotes ) {
				note = note.trimmed();
				if ( note.startsWith( '*' ) && note.endsWith( '*' ) ) {
					update_alerts.append( note.mid( 1, note.length() - 2 ) );
				} else {
					update_changes.append( note );
				}
			}

			// If this is greater than the highest version found so far...
			if ( major > update_major ||
			     ( major == update_major && minor > update_minor ) ||
			     ( major == update_major && minor == update_minor && revision > update_revision ) ) {
				update_version = version;
				update_major = major;
				update_minor = minor;
				update_revision = revision;
				update_url = entry_url;
			}
		}
	}

	if ( update_major != MAJOR_VERSION || update_minor != MINOR_VERSION || update_revision != REVISION ) {
		UpdateManager::instance()->updateFound( update_version, update_url, update_alerts, update_changes );
	} else if ( forceNotification ) {
		UpdateManager::instance()->noUpdateFound();
	}
}
