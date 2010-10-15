#include "serverconfigdlg.h"
#include "ui_serverconfigdlg.h"

ServerConfigDlg::ServerConfigDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerConfigDlg)
{
    ui->setupUi(this);
	connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(accept()));

	ui->login->setFocus();
}

ServerConfigDlg::~ServerConfigDlg()
{
    delete ui;
}

QString ServerConfigDlg::getHostname()
{
	return ui->hostName->text();
}

QString ServerConfigDlg::getLogin()
{
	return ui->login->text();
}

QString ServerConfigDlg::getPassword()
{
	return ui->password->text();
}

QString ServerConfigDlg::getFilename()
{
	return ui->filename->text();
}
