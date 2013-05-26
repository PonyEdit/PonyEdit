#include <QCoreApplication>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QDebug>
#include <QVariantMap>
#include <QDate>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include <QDesktopServices>

#include "sitemanager.h"
#include "tools/json.h"

SiteManager::SiteManager()
{
	mManager = new QNetworkAccessManager(this);

	#ifdef Q_OS_WIN
		mOS = "WIN";
	#endif
	#ifdef Q_OS_MAC
		mOS = "OSX";
	#endif
	#ifdef Q_OS_LINUX
		mOS = "LIN";
	#endif

	connect(mManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleReply(QNetworkReply*)));
}

SiteManager::~SiteManager()
{
	//	Clean out any leftover replies
	foreach (QNetworkReply* reply, mReplies.keys())
		delete reply;

	if(mManager)
		delete mManager;

	mManager = NULL;
}

void SiteManager::checkForUpdates(bool forceNotification)
{
	QString version = QString("vmaj=%1&vmin=%2&rev=%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(REVISION);

	QUrl url(QString(SITE_URL) + "version/?" + version);
	QNetworkReply* reply = mManager->get(QNetworkRequest(url));

	mReplies.insert(reply, forceNotification ? UpdateCheckForcedNotification : UpdateCheck);
}

void SiteManager::handleReply(QNetworkReply *reply)
{
	if (!mReplies.contains(reply)) return;
	Messages message = mReplies.value(reply);

	try
	{
		if (reply->error() != QNetworkReply::NoError)
			throw(reply->errorString());

		QByteArray result = reply->readAll();

		bool ok;
		QVariantMap data = Json::parse(result, ok).toMap();
		if (!ok) throw(tr("Failed to parse reply from website"));

		QVariantMap version;
		QVariantMap changes;
		switch(message)
		{
			case UpdateCheck:
			case UpdateCheckForcedNotification:
				version = data[mOS].toMap();
				changes = data["changes"].toMap();
				if(version["major"].toInt() > MAJOR_VERSION ||
						(version["major"].toInt() == MAJOR_VERSION && version["minor"].toInt() > MINOR_VERSION) ||
						(version["major"].toInt() == MAJOR_VERSION && version["minor"].toInt() == MINOR_VERSION && version["revision"].toInt() > REVISION))
					emit updateAvailable(version, changes);
				else if(message == UpdateCheckForcedNotification)
					emit noUpdateAvailable();
				break;
		}
	}
	catch (QString error)
	{
		switch(message)
		{
			case UpdateCheck:
			case UpdateCheckForcedNotification:
				//	Do nothing
				break;
		}
	}

	mReplies.remove(reply);
	reply->deleteLater();
}

void SiteManager::feedbackHappy()
{
	QUrl url(QString(SITE_URL) + "feedback/?feedback=happy&version=" + QCoreApplication::applicationVersion());

	QDesktopServices::openUrl(url);
}

void SiteManager::feedbackSad()
{
	QUrl url(QString(SITE_URL) + "feedback/?feedback=sad&version=" + QCoreApplication::applicationVersion());

	QDesktopServices::openUrl(url);
}

void SiteManager::purchase()
{
	QUrl url(QString(SITE_URL) + "buy/");

	QDesktopServices::openUrl(url);
}
