#ifndef SITEMANAGER_H
#define SITEMANAGER_H

#include <QByteArray>
#include <QList>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QVariantMap>

#define SITE_URL "http://ponyedit.com/"
#define CDN_URL "http://cdn.ponyedit.com/"

class SiteManager : public QObject {
	Q_OBJECT

public:
	enum Messages { UpdateCheck,
		UpdateCheckForcedNotification };
	SiteManager();
	~SiteManager();

public slots:
	void checkForUpdates( bool forceNotification = false );
	void handleReply( QNetworkReply *reply );

signals:
	void updateAvailable( const QVariantMap &version, const QVariantMap &changes );
	void noUpdateAvailable();

private:
	void handleUpdateCheckReply( QList< QVariant > reply, bool forceNotification );

	QNetworkAccessManager *           mManager;
	QMap< QNetworkReply *, Messages > mReplies;
};

extern SiteManager *gSiteManager;

#endif // SITEMANAGER_H
