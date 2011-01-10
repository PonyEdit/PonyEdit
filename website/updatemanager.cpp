#include <QMessageBox>
#include <QDebug>
#include <QVariantMap>
#include <QCoreApplication>

#include "sitemanager.h"
#include "updatemanager.h"
#include "updatenotificationdialog.h"

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
	connect(gSiteManager, SIGNAL(updateAvailable(const QString&, const QVariantMap&)), this, SLOT(updateFound(const QString&, const QVariantMap&)));
}

void UpdateManager::updateFound(const QString& version, const QVariantMap& changes)
{
	QString url = QString("%1downloads/").arg(SITE_URL);

	UpdateNotificationDialog dlg;

	QVariantMap relevantChanges;

	QMapIterator<QString, QVariant> ii(changes);
	while(ii.hasNext())
	{
		ii.next();
		if(ii.key() > QCoreApplication::applicationVersion())
			relevantChanges.insert(ii.key(), ii.value());
	}

	dlg.setNewVersion(version);
	dlg.setChanges(relevantChanges);
	dlg.setDownloadURL(url, url);

	dlg.exec();
}
