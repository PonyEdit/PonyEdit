#include "connectionstatuswidget.h"
#include "ui_statuswidget.h"
#include "remoteconnection.h"
#include "main/statuswidget.h"
#include <QDebug>
#include <QDialog>
#include <QPushButton>
#include <QTimer>

ConnectionStatusWidget::ConnectionStatusWidget(RemoteConnection* connection, bool modal, QWidget* parent) :
	StatusWidget(modal, parent)
{
	mConnection = connection;

	getButtonBox()->setStandardButtons(QDialogButtonBox::Cancel);
	getButtonBox()->button(QDialogButtonBox::Cancel)->setDefault(false);

	connect(connection, SIGNAL(statusChanged()), this, SLOT(connectionStatusChanged()), Qt::QueuedConnection);
	connect(this, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(onButtonClicked(QAbstractButton*)));
}

ConnectionStatusWidget::~ConnectionStatusWidget()
{
	clearExtraButtons();
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
		close(baseStatus == RemoteConnection::Disconnected);
		return;
	}

	if (status & RemoteConnection::WaitingOnInput)
	{
		if (!isShowingInput())
		{
			QWidget* inputWidget = new QWidget(this);
			mConnection->populateInputDialog(this, inputWidget);
			setInputWidget(inputWidget);
		}
	}
	else
	{
		clearInputWidget();
		setStatus(mConnection->getStatusIcon(), mConnection->getName() + ": " + mConnection->getStatusString());
	}
}

void ConnectionStatusWidget::addButton(QDialogButtonBox::ButtonRole role, const QString& label)
{
	QPushButton* button = getButtonBox()->addButton(label, role);
	mExtraButtons.push_back(button);
	button->setDefault(true);
	button->setFocus();
}

void ConnectionStatusWidget::onButtonClicked(QAbstractButton *button)
{
	QDialogButtonBox* buttonBox = getButtonBox();
	QDialogButtonBox::ButtonRole role = buttonBox->buttonRole(button);
	if (role == QDialogButtonBox::RejectRole)
	{
		buttonBox->setEnabled(false);

		RemoteConnection::Status status = mConnection->getBaseStatus();
		if (status != RemoteConnection::Disconnecting && status != RemoteConnection::Disconnected && status != RemoteConnection::Error)
			mConnection->disconnect(true);
		else
			close(false);
	}
	else if (mConnection->inputDialogCallback(this, role))
	{
		clearInputWidget();
		clearExtraButtons();
		setStatus(mConnection->getStatusIcon(), mConnection->getName() + ": " + mConnection->getStatusString());
		mConnection->inputDialogCompleted();
	}
}

void ConnectionStatusWidget::clearExtraButtons()
{
	foreach (QPushButton* b, mExtraButtons)
		b->deleteLater();
	mExtraButtons.clear();
}

















