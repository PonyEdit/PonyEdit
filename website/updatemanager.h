#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QAuthenticator>

#include "updatenotificationdialog.h"

class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = 0);

signals:

public slots:
	void updateFound(const QVariantMap& version, const QVariantMap& changes);
	void noUpdateFound();

	void startDownload(QString file);
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void downloadFinished();
	void downloadReadyRead();
	void downloadAuth(QNetworkReply * reply, QAuthenticator * authenticator);

private:

	UpdateNotificationDialog* mNotificationDlg;

	QNetworkAccessManager mNetManager;
	QNetworkReply *mDownload;

	QFile mTempFile;

};

#endif // UPDATEMANAGER_H
