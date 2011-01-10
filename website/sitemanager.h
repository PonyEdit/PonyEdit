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
	enum Messages {UpdateCheck, LicenceCheck};
    SiteManager();
	~SiteManager();

	void checkForUpdates();
	void checkLicence();

public slots:
	void handleReply(QNetworkReply* reply);

signals:
	void updateAvailable(const QString& version, const QVariantMap& changes);
	void licenceStatus(bool valid);

private:
	QNetworkAccessManager* mManager;
	QList<QNetworkReply*> mReplies;
	QList<Messages> mReplyTypes;
	QString mOS;

};

extern SiteManager* gSiteManager;

#endif // SITEMANAGER_H
