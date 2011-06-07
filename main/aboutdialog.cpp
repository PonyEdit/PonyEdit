#include <QCoreApplication>

#include "licence/licence.h"

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

	ui->versionLabel->setText(ui->versionLabel->text().arg(QCoreApplication::applicationVersion()).arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(REVISION));

	Licence l = Licence();

	QString login = l.getLogin();
	if(login.isEmpty())
		login = tr("Anonymous");

	QString details;
	if(!l.getExpiry().isNull())
		details = tr("Expires: %1").arg(l.getExpiry().toString(Qt::ISODate));
	else
		details = tr("Expires: (No Expiry)");

	details += "<br />";

	int version = l.getMaximumVersion();
	if(version <= 0)
		details += tr("Max Version: (No Maximum Version)");
	else
		details += tr("Max Version: %1").arg(version);

	details += "<br/>";

	details += tr("Key: ") + "<br/>";
	details += l.getKey();

	ui->licenceLabel->setText(ui->licenceLabel->text().arg(login));
	ui->licenceDetails->appendHtml(details);
	ui->licenceDetails->moveCursor(QTextCursor::Start);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
