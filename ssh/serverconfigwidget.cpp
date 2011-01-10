#include "serverconfigwidget.h"
#include "ui_serverconfigwidget.h"
#include "main/tools.h"
#include "ssh/sshhost.h"
#include <QFileDialog>

ServerConfigWidget::ServerConfigWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ServerConfigWidget)
{
	ui->setupUi(this);

	QIntValidator* portValidator = new QIntValidator(0, 65535, this);
	ui->hostPort->setValidator(portValidator);

	connect(this, SIGNAL(accepted()), this, SLOT(acceptedHandler()));
	connect(ui->hostName, SIGNAL(textEdited(QString)), this, SLOT(updateName()));
	connect(ui->userName, SIGNAL(textEdited(QString)), this, SLOT(updateName()));
	connect(ui->keyFileBrowse, SIGNAL(clicked()), this, SLOT(browseForKeyFile()));

	for (int i = 0; i < SshRemoteController::NumScriptTypes; i++)
		ui->scriptType->addItem(SshRemoteController::sScriptTypeLabels[i], QVariant::fromValue<int>(i));

	ui->password->setFocus();
}

ServerConfigWidget::~ServerConfigWidget()
{
	delete ui;
}

QString ServerConfigWidget::getHostName()
{
	return ui->hostName->text();
}

QString ServerConfigWidget::getUserName()
{
	return ui->userName->text();
}

QString ServerConfigWidget::getPassword()
{
	return ui->password->text();
}

void ServerConfigWidget::setEditHost(SshHost* host)
{
	mEditHost = host;
	ui->hostName->setText(host->getHostName());
	ui->userName->setText(host->getUserName());
	ui->hostPort->setText(QString::number(host->getPort()));
	ui->password->setText(host->getPassword());
	ui->savePassword->setChecked(host->getSavePassword());
	ui->saveServer->setChecked(host->getSave());
	ui->serverName->setText(host->getName());
	ui->defaultDirectory->setText(host->getDefaultDirectory());
	ui->keyFile->setText(host->getKeyFile());

	int scriptIndex = ui->scriptType->findData(QVariant::fromValue<int>((int)host->getScriptType()));
	if(scriptIndex < 0)
		scriptIndex = 0;
	ui->scriptType->setCurrentIndex(scriptIndex);

	mLastAutoName = getAutoName();
	updateName();
}

void ServerConfigWidget::acceptedHandler()
{
	updateName();

	mEditHost->setHostName(ui->hostName->text());
	mEditHost->setUserName(ui->userName->text());
	mEditHost->setPort(ui->hostPort->text().toInt());
	mEditHost->setPassword(ui->password->text());
	mEditHost->setSave(ui->saveServer->checkState() == Qt::Checked);
	mEditHost->setName(ui->serverName->text());
	mEditHost->setDefaultDirectory(ui->defaultDirectory->text());
	mEditHost->setSavePassword(ui->savePassword->checkState() == Qt::Checked);
	mEditHost->setScriptType((SshRemoteController::ScriptType)ui->scriptType->itemData(ui->scriptType->currentIndex()).value<int>());
	mEditHost->setKeyFile(ui->keyFile->text());
}

QString ServerConfigWidget::getAutoName()
{
	QString userName = ui->userName->text();
	QString hostName = ui->hostName->text();

	return (userName.isEmpty() ? "" : userName + "@") + hostName;
}

void ServerConfigWidget::updateName()
{
	QString currentName = ui->serverName->text();
	if (currentName.isEmpty() || currentName == mLastAutoName)
	{
		mLastAutoName = getAutoName();
		ui->serverName->setText(mLastAutoName);
	}
}

void ServerConfigWidget::browseForKeyFile()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Select an OpenSSH key file"), ui->keyFile->text());
	if (!filename.isEmpty())
	{
#ifdef Q_OS_WIN
		filename = filename.replace('/', '\\');
#endif

		ui->keyFile->setText(filename);
	}
}

