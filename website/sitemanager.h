#ifndef SITEMANAGER_H
#define SITEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QVariantMap>

#define SITE_URL "http://ponyedit.com/"

class SiteManager : public QObject
{
	Q_OBJECT

public:
	enum Messages {UpdateCheck, UpdateCheckForcedNotification, LicenceCheck, GetTrial};
    SiteManager();
	~SiteManager();

	void checkForUpdates(bool forceNotification = false);
	void checkLicence();
	void getTrial();

public slots:
	void handleReply(QNetworkReply* reply);

	void feedbackHappy();
	void feedbackSad();

	void purchase();

signals:
	void updateAvailable(const QString& version, const QVariantMap& changes);
	void noUpdateAvailable();
	void licenceStatus(bool valid);
	void gotTrial(const QString& licence);

private:
	QNetworkAccessManager* mManager;
	QList<QNetworkReply*> mReplies;
	QList<Messages> mReplyTypes;
	QString mOS;

};

extern SiteManager* gSiteManager;

#endif // SITEMANAGER_H
