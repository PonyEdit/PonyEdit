#include <QCoreApplication>
#include <QDesktopServices>

#include "updatenotificationdialog.h"
#include "ui_updatenotificationdialog.h"

UpdateNotificationDialog::UpdateNotificationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateNotificationDialog)
{
    ui->setupUi(this);

	ui->downloadProgress->hide();
	ui->downloadLabel->hide();

	ui->changesBrowser->setOpenExternalLinks(true);

	connect(ui->installLater, SIGNAL(clicked()), this, SLOT(reject()));
	connect(ui->downloadSite, SIGNAL(clicked()), this, SLOT(openDownloadURL()));
	connect(ui->installNow, SIGNAL(clicked()), this, SLOT(emitDownloadAndInstall()));
}

UpdateNotificationDialog::~UpdateNotificationDialog()
{
    delete ui;
}

void UpdateNotificationDialog::setNewVersion(const QString &version)
{
	QString label = ui->updateLabel->text().arg(QCoreApplication::applicationVersion(), version);
	ui->updateLabel->setText(label);
}

void UpdateNotificationDialog::setChanges(const QVariantMap &changes)
{
	QString changesStr;
	QMapIterator<QString, QVariant> ii(changes);
	while(ii.hasNext())
	{
		ii.next();
		changesStr += "<strong>";
		changesStr += tr("Version %1:").arg(ii.key());
		changesStr += "</strong><br />";
		changesStr += ii.value().toString();
		changesStr += "<hr />";
	}

	ui->changesBrowser->setHtml(changesStr);
}

void UpdateNotificationDialog::setDownloadURL(const QString &downloadURL, const QString& fileURL)
{
	mDownloadURL = downloadURL;
	mFileURL = fileURL;
}

void UpdateNotificationDialog::emitDownloadAndInstall()
{
	emit downloadAndInstall(mFileURL);
}

void UpdateNotificationDialog::openDownloadURL()
{
	QDesktopServices::openUrl(QUrl(mDownloadURL));
}

QProgressBar* UpdateNotificationDialog::getProgressBar()
{
	return ui->downloadProgress;
}

QLabel* UpdateNotificationDialog::getProgressLabel()
{
	return ui->downloadLabel;
}

QWidget* UpdateNotificationDialog::getButtonWrapper()
{
	return ui->buttonWrapper;
}
