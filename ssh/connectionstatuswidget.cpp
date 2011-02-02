#include "connectionstatuswidget.h"
#include "ui_connectionstatuswidget.h"
#include "remoteconnection.h"
#include <QDebug>
#include <QDialog>
#include <QPushButton>
#include <QTimer>

ConnectionStatusWidget::ConnectionStatusWidget(RemoteConnection* connection, bool modal, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::ConnectionStatusWidget)
{
	mConnection = connection;
	mCurrentInputWidget = NULL;
	mModal = modal;

    ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Cancel)->setDefault(false);

	connect(this, SIGNAL(signalUpdateLayouts()), this, SLOT(updateLayouts()), Qt::QueuedConnection);
	connect(connection, SIGNAL(statusChanged()), this, SLOT(connectionStatusChanged()), Qt::QueuedConnection);
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

ConnectionStatusWidget::~ConnectionStatusWidget()
{
	clearExtraButtons();
    delete ui;
}

void ConnectionStatusWidget::showEvent(QShowEvent*)
{
	connectionStatusChanged();
}

void ConnectionStatusWidget::connectionStatusChanged()
{
	RemoteConnection::Status status = mConnection->getStatus();
	RemoteConnection::Status baseStatus = mConnection->getBaseStatus();

	//	Work out if we're through showing this dialog
	if (baseStatus == (mConnection->isDeliberatelyDisconnecting() ? RemoteConnection::Disconnected : RemoteConnection::Connected))
	{
		if (mModal)
		{
			QDialog* dialogParent = static_cast<QDialog*>(parentWidget());
			if (baseStatus == RemoteConnection::Disconnected)
				dialogParent->reject();
			else
				dialogParent->accept();
		}
		emit completed();
		delete this;
		return;
	}

	//	show input widget, or hide it / update it.
	if (status & RemoteConnection::WaitingOnInput)
		showInput();
	else
		hideInput();
}

void ConnectionStatusWidget::setManualStatus(QString text, QPixmap icon)
{
	ui->statusLabel->setText(mConnection->getName() + ": " + text);
	ui->statusIcon->setPixmap(icon);
}

void ConnectionStatusWidget::addButton(QDialogButtonBox::ButtonRole role, const QString& label)
{
	QPushButton* button = ui->buttonBox->addButton(label, role);
	mExtraButtons.push_back(button);
	button->setDefault(true);
	button->setFocus();
}

void ConnectionStatusWidget::showInput()
{
	mCurrentInputWidget = new QWidget(this);
	mConnection->populateInputDialog(this, mCurrentInputWidget);
	ui->childArea->addWidget(mCurrentInputWidget);

	emit signalUpdateLayouts();

	if (hasFocus())
		mCurrentInputWidget->setFocus();
}

void ConnectionStatusWidget::updateLayouts()
{
	setMinimumSize(layout()->minimumSize());
	parentWidget()->adjustSize();
}

void ConnectionStatusWidget::clearExtraButtons()
{
	foreach (QPushButton* b, mExtraButtons)
		b->deleteLater();
	mExtraButtons.clear();
}

void ConnectionStatusWidget::hideInput()
{
	clearExtraButtons();

	mConnection->inputDialogCompleted();
	if (mCurrentInputWidget != NULL)
	{
		delete mCurrentInputWidget;
		mCurrentInputWidget = NULL;
	}

	ui->statusLabel->setText(mConnection->getName() + ": " + mConnection->getStatusString());
	ui->statusIcon->setPixmap(mConnection->getStatusIcon());
}

void ConnectionStatusWidget::buttonClicked(QAbstractButton *button)
{
	QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(button);
	if (role == QDialogButtonBox::RejectRole)
	{
		ui->buttonBox->setEnabled(false);

		RemoteConnection::Status status = mConnection->getBaseStatus();
		if (status != RemoteConnection::Disconnecting && status != RemoteConnection::Disconnected && status != RemoteConnection::Error)
			mConnection->disconnect(true);
		else if (mModal)
			static_cast<QDialog*>(parentWidget())->reject();

		hideInput();
	}
	else if (mCurrentInputWidget != NULL && mConnection->inputDialogCallback(this, role))
		hideInput();
}

















