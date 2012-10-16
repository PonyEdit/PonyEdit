#include "offlineactivationdialog.h"
#include "ui_offlineactivationdialog.h"
#include "licence.h"
#include <QPushButton>

OfflineActivationDialog::OfflineActivationDialog(QWidget *parent) :
    QDialog(parent),
	ui(new Ui::OfflineActivationDialog),
	mLicence("")
{
    ui->setupUi(this);
	ui->licenceKeyEntry->setFocus();

	connect(ui->licenceKeyEntry, SIGNAL(textChanged()), this, SLOT(validate()));
	validate();
}

OfflineActivationDialog::~OfflineActivationDialog()
{
    delete ui;
}

void OfflineActivationDialog::validate()
{
	QString key = ui->licenceKeyEntry->document()->toPlainText();

    mLicence.setKey(key.toLatin1());
	ui->validLicenceIcon->setVisible(mLicence.isValid());
	ui->validLicenceLabel->setVisible(mLicence.isValid());
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(mLicence.isValid());
}

void OfflineActivationDialog::accept()
{
	if (mLicence.isValid())
		QDialog::accept();
}











