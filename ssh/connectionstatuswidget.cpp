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

	setButtons(Cancel);

	connect(connection, SIGNAL(statusChanged()), this, SLOT(connectionStatusChanged()), Qt::QueuedConnection);
	connect(this, SIGNAL(buttonClicked(StatusWidget::Button)), this, SLOT(onButtonClicked(StatusWidget::Button)));
}

ConnectionStatusWidget::~ConnectionStatusWidget()
{
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
		setButtons(Cancel);
		clearInputWidget();
		setStatus(mConnection->getStatusIcon(), mConnection->getName() + ": " + mConnection->getStatusString());

		if (status & RemoteConnection::Error)
		{
			setButtons(Done);
			this->setButtonsEnabled(true);
		}
	}
}

void ConnectionStatusWidget::onButtonClicked(StatusWidget::Button button)
{
	if (button == Cancel)
	{
		RemoteConnection::Status status = mConnection->getBaseStatus();
		if (status != RemoteConnection::Disconnecting && status != RemoteConnection::Disconnected && status != RemoteConnection::Error)
		{
			setButtonsEnabled(false);
			mConnection->disconnect(true);
		}
		else
			close(false);
	}
	else if (button == Done)
		close(false);
	else if (mConnection->inputDialogCallback(this, button))
	{
		setButtons(Cancel);
		clearInputWidget();
		setStatus(mConnection->getStatusIcon(), mConnection->getName() + ": " + mConnection->getStatusString());
		mConnection->inputDialogCompleted();
	}
}


















