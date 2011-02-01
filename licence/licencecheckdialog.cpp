#include "licence.h"
#include "website/sitemanager.h"

#include "licencecheckdialog.h"
#include "ui_licencecheckdialog.h"

LicenceCheckDialog::LicenceCheckDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LicenceCheckDialog)
{
    ui->setupUi(this);

	connect(ui->buyPonyEditButton, SIGNAL(clicked()), gSiteManager, SLOT(purchase()));
	connect(ui->trialActivate, SIGNAL(clicked()), this, SLOT(getTrial()));
	connect(gSiteManager, SIGNAL(gotTrial(QString)), this, SLOT(saveTrial(QString)));
}

LicenceCheckDialog::~LicenceCheckDialog()
{
    delete ui;
}

void LicenceCheckDialog::getTrial()
{
	gSiteManager->getTrial();
}

void LicenceCheckDialog::saveTrial(const QString &key)
{
	qDebug() << "Licence key:" << key.toUtf8();
	Licence l(key.toUtf8());
	qDebug() << "Licence is valid: " << l.isValid();
	qDebug() << "Licence has expired: " << l.hasExpired();
	qDebug() << "Licence expiry date: " << l.getExpiry();
	qDebug() << "Licence login name: " << l.getLogin();
	qDebug() << "Licence max version: " << l.getMaximumVersion();
	qDebug() << "Licence issue id: " << l.getIssueId();
}
