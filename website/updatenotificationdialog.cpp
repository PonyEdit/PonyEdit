#include <QCoreApplication>

#include "updatenotificationdialog.h"
#include "ui_updatenotificationdialog.h"

UpdateNotificationDialog::UpdateNotificationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpdateNotificationDialog)
{
    ui->setupUi(this);

	ui->changesBrowser->setOpenExternalLinks(true);
	ui->urlLabel->setOpenExternalLinks(true);
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

void UpdateNotificationDialog::setDownloadURL(const QString &href, const QString &display)
{
	QString label = ui->urlLabel->text().arg(href, display);
	ui->urlLabel->setText(label);
}
