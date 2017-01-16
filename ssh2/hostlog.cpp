HIDE_COMPILE_WARNINGS

#include "ui_hostlog.h"

UNHIDE_COMPILE_WARNINGS

#include "hostlog.h"
#include "sshhost.h"

HostLog::HostLog(SshHost* host) :
	QWidget(NULL),
	ui(new Ui::HostLog)
{
	ui->setupUi(this);
	ui->textBox->setPlainText(host->getLog().join("\n"));
	connect(host, SIGNAL(newLogLine(QString)), this, SLOT(newLogLine(QString)));

	setWindowTitle("Log: " + host->getName());
}

HostLog::~HostLog()
{
	delete ui;
}

void HostLog::closeEvent(QCloseEvent*)
{
	this->deleteLater();
}

void HostLog::newLogLine(QString line)
{
	ui->textBox->appendPlainText(line);
}
