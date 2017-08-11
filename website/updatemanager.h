#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QAuthenticator>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QVariantMap>

#include "updatenotificationdialog.h"

class UpdateManager : public QObject {
	Q_OBJECT

	public:
		explicit UpdateManager( QObject *parent = 0 );

		static UpdateManager* instance() {
			return sInstance;
		}

		void updateFound( const QString& version, const QString& url, const QStringList& alerts, const QStringList& changes );
		void noUpdateFound();

	signals:
	public slots:
		void startDownload( QString file );
		void downloadProgress( qint64 bytesReceived, qint64 bytesTotal );
		void downloadFinished();
		void downloadReadyRead();
		void downloadAuth( QNetworkReply * reply, QAuthenticator * authenticator );

	private:
		static UpdateManager* sInstance;

		UpdateNotificationDialog* mNotificationDlg;

		QNetworkAccessManager mNetManager;
		QNetworkReply *mDownload;
		int mRedirectCount;

		QFile mTempFile;
};

#endif  // UPDATEMANAGER_H
