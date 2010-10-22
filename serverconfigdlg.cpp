#include "serverconfigdlg.h"
#include "ui_serverconfigdlg.h"

ServerConfigDlg::ServerConfigDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerConfigDlg)
{
    ui->setupUi(this);

	QIntValidator* portValidator = new QIntValidator(0, 65535, this);
	ui->hostPort->setValidator(portValidator);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(this, SIGNAL(accepted()), this, SLOT(acceptedHandler()));
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
	ui->hostPort->setText(QString::number(host->getPort()));
	ui->password->setText(host->getPassword());
}

void ServerConfigDlg::acceptedHandler()
{
	mEditHost->setHostName(ui->hostName->text());
	mEditHost->setUserName(ui->userName->text());
	mEditHost->setPort(ui->hostPort->text().toInt());
	mEditHost->setPassword(ui->password->text());
}

