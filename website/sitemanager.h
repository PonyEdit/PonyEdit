#ifndef SITEMANAGER_H
#define SITEMANAGER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QVariantMap>
#include <QMap>

#define SITE_URL "http://ponyedit.com/"
#define CDN_URL "http://cdn.ponyedit.com/"

class SiteManager : public QObject
{
	Q_OBJECT

public:
	enum Messages {UpdateCheck, UpdateCheckForcedNotification, LicenceCheck, GetTrial, GetLicence};
    SiteManager();
	~SiteManager();

	void getLicence(const QString& username, const QString& password);
	void checkLicence();
	void getTrial();

public slots:
	void checkForUpdates(bool forceNotification = false);
	void handleReply(QNetworkReply* reply);

	void feedbackHappy();
	void feedbackSad();

	void purchase();

signals:
	void updateAvailable(const QVariantMap& version, const QVariantMap& changes);
	void noUpdateAvailable();
	void licenceStatus(bool valid);
	void getTrial(const QString& licence);
	void getTrialFailed(const QString& error);
	void getLicence(const QString& licence);
	void getLicenceFailed(const QString& error);

private:
	QNetworkAccessManager* mManager;
	QMap<QNetworkReply*, Messages> mReplies;
	QString mOS;

};

extern SiteManager* gSiteManager;

#endif // SITEMANAGER_H
