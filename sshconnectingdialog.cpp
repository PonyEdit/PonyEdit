#include "sshconnectingdialog.h"
#include "ui_sshconnectingdialog.h"
#include "passworddialog.h"
#include <QDebug>

SshConnectingDialog::SshConnectingDialog(SshHost* host, SshRemoteController* controller) :
	QDialog(0, Qt::CustomizeWindowHint | Qt::WindowTitleHint),
	ui(new Ui::SshConnectingDialog)
{
    ui->setupUi(this);
	mHost = host;
	mController = controller;
	mLastStatusChange = -1;

	mTimer = new QTimer();
	connect(mTimer, SIGNAL(timeout()), this, SLOT(tick()));
}

SshConnectingDialog::~SshConnectingDialog()
{
    delete ui;
	delete mTimer;
}

void SshConnectingDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent(event);

	mTimer->setSingleShot(false);
	mTimer->start(100);
}

void SshConnectingDialog::tick()
{
	int latestStatus = mController->getLastStatusChange();
	if (latestStatus > mLastStatusChange)
	{
		mLastStatusChange = latestStatus;
		SshRemoteController::Status status = mController->getStatus();
		QString statusText = SshRemoteController::getStatusString(status);
		statusText[0] = statusText.at(0).toUpper();
		statusText += " ... ";
		ui->status->setText(statusText);

		if (status == SshRemoteController::WaitingForPassword)
		{
			QString password = PasswordDialog::getPassword(this, QString("Please enter your SSH password for ") + mHost->getName());
			if (password.isEmpty())
				cancel();
			mHost->setPassword(password);
		}
		else if (status == SshRemoteController::Connected)
			accept();
	}
}

void SshConnectingDialog::cancel()
{
	qDebug() << "Cancel.";
}

