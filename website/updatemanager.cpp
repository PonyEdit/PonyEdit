#include <QMessageBox>
#include <QDebug>
#include <QVariantMap>
#include <QCoreApplication>
#include <QDir>
#include <QStringList>
#include <QProcess>
#include <QApplication>
#include <QUuid>

#include "sitemanager.h"
#include "updatemanager.h"
#include "updatenotificationdialog.h"
#include "file/openfilemanager.h"

UpdateManager::UpdateManager(QObject *parent) :
    QObject(parent)
{
	connect(gSiteManager, SIGNAL(updateAvailable(const QVariantMap&, const QVariantMap&)), this, SLOT(updateFound(const QVariantMap&, const QVariantMap&)));
	connect(gSiteManager, SIGNAL(noUpdateAvailable()), this, SLOT(noUpdateFound()));
}

void UpdateManager::updateFound(const QVariantMap& version, const QVariantMap& changes)
{
	QString url = QString("%1download/").arg(CDN_URL);
	QString ext;
#ifdef Q_OS_WIN
	ext = "exe";
#elif defined Q_OS_MAC
	ext = "dmg";
#endif

	QString fullURL = QString("%1files/PonyEdit-%2.%3").arg(CDN_URL, version["pretty"].toString(), ext);

	mNotificationDlg = new UpdateNotificationDialog();

	connect(mNotificationDlg, SIGNAL(downloadAndInstall(QString)), this, SLOT(startDownload(QString)));

	mNotificationDlg->setNewVersion(version["pretty"].toString());
	mNotificationDlg->setChanges(changes);
	mNotificationDlg->setDownloadURL(url, fullURL);

	mNotificationDlg->exec();
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

void UpdateManager::startDownload(QString file)
{
	QProgressBar* progressBar = mNotificationDlg->getProgressBar();
	progressBar->show();

	QLabel* progressLabel = mNotificationDlg->getProgressLabel();
	progressLabel->show();

	QWidget* buttonWrapper = mNotificationDlg->getButtonWrapper();
	buttonWrapper->hide();

	connect(&mNetManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(downloadAuth(QNetworkReply*,QAuthenticator*)));

	QUrl download(file);

	mTempFile.setFileName(QDir::tempPath() + "/" + QFileInfo(download.path()).fileName());
	mTempFile.open(QIODevice::WriteOnly);

	QNetworkRequest request(download);

	mDownload = mNetManager.get(request);

	connect(mDownload, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
	connect(mDownload, SIGNAL(finished()), this, SLOT(downloadFinished()));
	connect(mDownload, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
}

void UpdateManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	QProgressBar* progressBar = mNotificationDlg->getProgressBar();
	progressBar->setMaximum(bytesTotal);
	progressBar->setValue(bytesReceived);

	QLabel* progressLabel = mNotificationDlg->getProgressLabel();

	double rec = bytesReceived;
	double total = bytesTotal;

	QStringList units;
	units << "B" << "KiB" << "MiB";
	int unit;

	for(unit = 0; rec > 1024 && unit < 2; unit++)
		rec /= 1024;

	total /= 1024*1024;

	progressLabel->setText(QString("%1 %2 of %3 MiB").arg(rec, 0, 'f', 1).arg(units[unit]).arg(total, 0, 'f', 1));
}

void UpdateManager::downloadFinished()
{
	mTempFile.close();

	if(mDownload->error() != QNetworkReply::NoError)
	{
		QMessageBox msg;
		msg.setWindowTitle(tr("Update unavailable"));
		msg.setText(tr("There was a problem downloading the update."));
		msg.setInformativeText(tr("Please try again later. If this error persists, please let us know on the <a href='%1'>support forum.</a>").arg(QString(SITE_URL) + "forums/"));
		msg.setStandardButtons(QMessageBox::Ok);

		msg.exec();

		mNotificationDlg->reject();

		return;
	}

	if (!gOpenFileManager.closeAllFiles())
		return;

	QString cmd;
	QStringList args;

	QLabel* progressLabel = mNotificationDlg->getProgressLabel();

	QFileInfo info(mTempFile);

	QProcess *installProc = new QProcess();

#ifdef Q_OS_WIN
	progressLabel->setText(tr("Installing..."));
	cmd = info.filePath();
	args << "/verysilent" << "/suppressmsgboxes" << "/norestart";

	installProc->startDetached(cmd, args);
#elif defined Q_OS_MAC
	progressLabel->setText(tr("Extracting..."));
	QUuid uuid;
	uuid.createUuid();

	QString mnt = "/Volumes/" + uuid.toString();

	cmd = "hdiutil";
	args << "attach" << info.filePath() << "-mountpoint" << mnt << "-noverify" << "-nobrowse" << "-noautoopen";

	installProc->execute(cmd, args);

	progressLabel->setText(tr("Installing..."));

	cmd = "rm";
	args.clear();
	args << "-rf" << "/Applications/PonyEdit.app";

	installProc->execute(cmd, args);

	cmd = "cp";
	args.clear();
	args << "-rf" << mnt + "/PonyEdit.app" << "/Applications";

	installProc->execute(cmd, args);

	cmd = "hdiutil";
	args.clear();
	args << "detach" << mnt << "-force";

	installProc->startDetached(cmd, args);

	progressLabel->setText(tr("Restarting..."));

	cmd = "open";
	args.clear();
	args << "/Applications/PonyEdit.app";

	installProc->startDetached(cmd, args);
#endif

	QApplication::exit();
}

void UpdateManager::downloadReadyRead()
{
	mTempFile.write(mDownload->readAll());
}

void UpdateManager::downloadAuth(QNetworkReply */* reply */, QAuthenticator *authenticator)
{
	authenticator->setUser("prealpha");
	authenticator->setPassword("letmein");
}
