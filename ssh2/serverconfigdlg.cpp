HIDE_COMPILE_WARNINGS

#include <QFileDialog>
#include <QGridLayout>

#include "ui_serverconfigdlg.h"

UNHIDE_COMPILE_WARNINGS

#include "serverconfigdlg.h"
#include "main/tools.h"
#include "sshhost.h"

ServerConfigDlg::ServerConfigDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ServerConfigDlg),
    mConfigWidget(new ServerConfigWidget())
{
    ui->setupUi(this);

	QGridLayout *layout = new QGridLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(mConfigWidget);

	ui->configContainer->setLayout(layout);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	connect(mConfigWidget, SIGNAL(accepted()), this, SLOT(accept()));
	connect(mConfigWidget, SIGNAL(rejected()), this, SLOT(reject()));

	connect(this, SIGNAL(accepted()), mConfigWidget, SLOT(acceptedHandler()));

	mConfigWidget->setFocus();
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
