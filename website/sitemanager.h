#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QNetworkAccessManager>

#ifndef SITEMANAGER_H
#define SITEMANAGER_H

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
	void updateAvailable(const QByteArray& version);
	void licenceStatus(bool valid);

private:
	QNetworkAccessManager* mManager;
	QList<QNetworkReply*> mReplies;
	QList<Messages> mReplyTypes;
	QString mOS;

};

#endif // SITEMANAGER_H
