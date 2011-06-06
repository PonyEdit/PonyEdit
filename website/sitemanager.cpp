#include <QCoreApplication>
#include <QUrl>
#include <QNetworkReply>
#include <QDebug>
#include <QVariantMap>
#include <QDate>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include <QDesktopServices>

#include "sitemanager.h"
#include "main/json.h"
#include "licence/licence.h"

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
	Licence l = Licence();
	QString version = QString("vmaj=%1&vmin=%2&rev=%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(REVISION);

	QUrl url(QString(SITE_URL) + "version/?" + version + "&u=" + l.getLogin() + "&key=" + l.getKey());
	QNetworkReply* reply = mManager->get(QNetworkRequest(url));

	mReplies.insert(reply, forceNotification ? UpdateCheckForcedNotification : UpdateCheck);
}

void SiteManager::getLicence(const QString &username, const QString &password)
{
	Licence l = Licence();
	QString version = QString("vmaj=%1&vmin=%2&rev=%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(REVISION);

	QUrl url(QString(SITE_URL) + "licence/?f=getlicence&" + version);
	url.addQueryItem("u", username);
	url.addQueryItem("p", password);
	QNetworkReply* reply = mManager->get(QNetworkRequest(url));

	mReplies.insert(reply, GetLicence);
}

void SiteManager::checkLicence()
{
	Licence l = Licence();
	QString version = QString("vmaj=%1&vmin=%2&rev=%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(REVISION);

	QUrl url(QString(SITE_URL) + "licence/?" + version + "&u=" + l.getLogin() + "&key=" + l.getKey());
	QNetworkReply* reply = mManager->get(QNetworkRequest(url));

	mReplies.insert(reply, LicenceCheck);
}

void SiteManager::getTrial()
{
	QUrl url(QString(SITE_URL) + "licence/?f=trial");
	QNetworkReply* reply = mManager->get(QNetworkRequest(url));

	mReplies.insert(reply, GetTrial);
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
		if (!ok) throw("Failed to parse reply from website");

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

			case LicenceCheck:
				emit licenceStatus(data["valid"].toBool());
				break;

			case GetTrial:
				emit getTrial(data["key"].toString());
				break;

			case GetLicence:
				if(data["key"].toString().isEmpty())
					emit getLicenceFailed(data["error"].toString());
				else
					emit getLicence(data["key"].toString());
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

			case LicenceCheck:
				//	Do nothing
				break;

			case GetTrial:
				emit getTrialFailed(error);
				break;

			case GetLicence:
				emit getLicenceFailed(error);
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
