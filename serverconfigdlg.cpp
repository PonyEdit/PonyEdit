#include "serverconfigdlg.h"
#include "ui_serverconfigdlg.h"

ServerConfigDlg::ServerConfigDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerConfigDlg)
{
    ui->setupUi(this);
	ui->userName->setFocus();
}

ServerConfigDlg::~ServerConfigDlg()
{
    delete ui;
}

QString ServerConfigDlg::getHostName()
{
	return ui->hostName->text();
}

QString ServerConfigDlg::getUserName()
{
	return ui->userName->text();
}

QString ServerConfigDlg::getPassword()
{
	return ui->password->text();
}

void ServerConfigDlg::setEditHost(SshHost* host)
{
	mEditHost = host;
	ui->hostName->setText(host->getHostName());
	ui->userName->setText(host->getUserName());
}

