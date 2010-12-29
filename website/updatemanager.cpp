#include <QMessageBox>
#include <QDebug>

#include "sitemanager.h"
#include "updatemanager.h"

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
	qDebug() << "update manager: " << connect(gSiteManager, SIGNAL(updateAvailable(const QByteArray&)), this, SLOT(updateFound(const QByteArray&)));
}

void UpdateManager::updateFound(const QByteArray& /* version */)
{
	QString url = QString("%1downloads/").arg(SITE_URL);

	QMessageBox msgBox;
	msgBox.setText(tr("There is an update available."));
	msgBox.setInformativeText(tr("You can download an updated version from:<br/><a href='%1'>%2</a>").arg(url, url));
	msgBox.setStandardButtons(QMessageBox::Ok);

	msgBox.exec();
}
