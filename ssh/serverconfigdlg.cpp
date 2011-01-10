#include "ssh/serverconfigdlg.h"
#include "ui_serverconfigdlg.h"
#include "main/tools.h"
#include "ssh/sshhost.h"
#include <QFileDialog>
#include <QGridLayout>

ServerConfigDlg::ServerConfigDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerConfigDlg)
{
    ui->setupUi(this);

	mConfigWidget = new ServerConfigWidget();

	QGridLayout *layout = new QGridLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(mConfigWidget);

	ui->configContainer->setLayout(layout);
	ui->configContainer->setFocusProxy(mConfigWidget);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	connect(this, SIGNAL(accepted()), mConfigWidget, SLOT(acceptedHandler()));

	ui->configContainer->setFocus();
}

ServerConfigDlg::~ServerConfigDlg()
{
    delete ui;
	delete mConfigWidget;
}

void ServerConfigDlg::setEditHost(SshHost *host)
{
	mConfigWidget->setEditHost(host);
}
