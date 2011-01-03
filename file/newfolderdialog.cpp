#include "newfolderdialog.h"
#include "ui_newfolderdialog.h"

NewFolderDialog::NewFolderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewFolderDialog)
{
    ui->setupUi(this);
}

NewFolderDialog::~NewFolderDialog()
{
    delete ui;
}

QString NewFolderDialog::folderName()
{
	QString name = ui->folderName->text();
	return name;
}
