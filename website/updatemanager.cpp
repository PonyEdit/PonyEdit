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
	connect(gSiteManager, SIGNAL(noUpdateAvailable()), this, SLOT(noUpdateFound()));
}

void UpdateManager::updateFound(const QString& version, const QVariantMap& changes)
{
	QString url = QString("%1download/").arg(SITE_URL);

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

void UpdateManager::noUpdateFound()
{
	QMessageBox msg;
	msg.setWindowTitle(tr("No updates available."));
	msg.setText(tr("No updates available."));
	msg.setInformativeText(tr("Your installation of PonyEdit is up to date."));
	msg.setStandardButtons(QMessageBox::Ok);

	msg.exec();
}
