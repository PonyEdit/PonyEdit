#include "passworddlg.h"
#include "ui_passworddlg.h"

PasswordDlg::PasswordDlg(QWidget *parent) : ThreadCrossingDialog(parent), ui(new Ui::PasswordDlg)
{
    ui->setupUi(this);
	ui->password->setFocus();
}

PasswordDlg::~PasswordDlg()
{
    delete ui;
}

void PasswordDlg::setOptions(const QVariantMap &options)
{
	setWindowTitle(options.value("title", "Enter your password").toString());
	ui->blurb->setText(options.value("blurb", "Please enter your password, and click OK to continue").toString());
	ui->label->setText(options.value("label", "Password: ").toString());
	ui->remember->setVisible(options.value("memorable", false).toBool());
	ui->remember->setChecked(options.value("remember", false).toBool());
}

QVariantMap PasswordDlg::getResult()
{
	QVariantMap result = ThreadCrossingDialog::getResult();
	result.insert("password", ui->password->text());
	result.insert("remember", ui->remember->isChecked());
	return result;
}
