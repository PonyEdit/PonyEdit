#include <QCoreApplication>

#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

	ui->versionLabel->setText(ui->versionLabel->text().arg(QCoreApplication::applicationVersion()));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
