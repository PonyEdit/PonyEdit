#include "passworddialog.h"
#include "ui_passworddialog.h"

PasswordDialog::PasswordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PasswordDialog)
{
    ui->setupUi(this);
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}

QString PasswordDialog::getPassword(QWidget* parent, const QString& prompt)
{
	PasswordDialog dlg(parent);
	dlg.ui->promptLine->setText(prompt);
	if (dlg.exec())
		return dlg.ui->password->text();
	else
		return QString();
}
