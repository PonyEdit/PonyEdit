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
	enum Messages {UpdateCheck, UpdateCheckForcedNotification};
    SiteManager();
	~SiteManager();

	SiteManager(SiteManager const&) = delete;
	SiteManager& operator=(SiteManager const&) = delete;

public slots:
	void checkForUpdates(bool forceNotification = false);
	void handleReply(QNetworkReply* reply);

signals:
	void updateAvailable(const QVariantMap& version, const QVariantMap& changes);
	void noUpdateAvailable();

private:
	void handleUpdateCheckReply(QList<QVariant> reply, bool forceNotification);

	QNetworkAccessManager* mManager;
	QMap<QNetworkReply*, Messages> mReplies;
};

extern SiteManager* gSiteManager;

#endif // SITEMANAGER_H
