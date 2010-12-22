#include <QCoreApplication>
#include <QUrl>
#include <QNetworkReply>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

#include "sitemanager.h"

#define SITE_URL "http://localhost/remoted/"

SiteManager::SiteManager()
{
	mManager = new QNetworkAccessManager(this);

	#ifdef Q_OS_WIN
		mOS = "WIN";
	#endif
	#ifdef Q_OS_MAC
		mOS = "MAC";
	#endif
	#ifdef Q_OS_LINUX
		mOS = "LIN";
	#endif;

	connect(mManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleReply(QNetworkReply*)));
}

SiteManager::~SiteManager()
{
	if(mManager)
		delete mManager;

	mManager = NULL;
}

void SiteManager::checkForUpdates()
{
	QUrl url(QString(SITE_URL) + "version/");
	QNetworkReply* reply = mManager->get(QNetworkRequest(url));

	mReplies.append(reply);
	mReplyTypes.append(UpdateCheck);
}

void SiteManager::checkLicence()
{
	QUrl url(QString(SITE_URL) + "licence/");
	QNetworkReply* reply = mManager->get(QNetworkRequest(url));

	mReplies.append(reply);
	mReplyTypes.append(LicenceCheck);
}

void SiteManager::handleReply(QNetworkReply *reply)
{
	int index = mReplies.indexOf(reply);
	if(index >= 0 && reply->error() == QNetworkReply::NoError)
	{
		QByteArray result = reply->readAll();

		QScriptValue data;
		QScriptEngine engine;

		data = engine.evaluate(QString(result));

		QString version;

		switch(mReplyTypes[index])
		{
			case UpdateCheck:
				version = data.property(mOS).toString();
				if(version > QCoreApplication::applicationVersion())
					emit updateAvailable(version.toUtf8());
				break;
			case LicenceCheck:
				break;
		}

		mReplies.removeAt(index);
		mReplyTypes.removeAt(index);
	}

	reply->deleteLater();
}
