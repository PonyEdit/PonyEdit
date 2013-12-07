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

void UpdateNotificationDialog::setChanges(const QStringList &alerts, const QStringList &changes)
{
	QString changesStr;

	if ( alerts.length() > 0 ) {
		foreach ( const QString& alert, alerts ) {
			changesStr += "<h4>" + alert + "</h4>";
		}
	}

	changesStr += "<ul>";
	foreach ( const QString& change, changes ) {
		changesStr += "<li>" + change + "</li>";
	}
	changesStr += "</ul>";
	ui->changesBrowser->setHtml( changesStr );
}

void UpdateNotificationDialog::setDownloadURL(const QString& fileURL)
{
	mFileURL = fileURL;
}

void UpdateNotificationDialog::emitDownloadAndInstall()
{
	emit downloadAndInstall(mFileURL);
}

void UpdateNotificationDialog::openDownloadURL()
{
	QDesktopServices::openUrl(QUrl("https://github.com/PonyEdit/PonyEdit/releases"));
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
