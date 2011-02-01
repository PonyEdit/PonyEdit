#include "licencecheckdialog.h"
#include "ui_licencecheckdialog.h"

LicenceCheckDialog::LicenceCheckDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LicenceCheckDialog)
{
    ui->setupUi(this);
}

LicenceCheckDialog::~LicenceCheckDialog()
{
    delete ui;
}
