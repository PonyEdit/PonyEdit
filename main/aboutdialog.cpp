HIDE_COMPILE_WARNINGS

#include <QCoreApplication>

#include "ui_aboutdialog.h"

UNHIDE_COMPILE_WARNINGS

#include "aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

	ui->versionLabel->setText(ui->versionLabel->text().arg(QCoreApplication::applicationVersion()).arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(REVISION));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
