#include "requeststatuswidget.h"
#include "ui_statuswidget.h"
#include "main/statuswidget.h"
#include "remoterequest.h"
#include "remotechannel.h"
#include "main/tools.h"
#include <QDialog>

RequestStatusWidget::RequestStatusWidget(RemoteChannel* channel, RemoteRequest* request, QString description, bool allowSudo) :
	StatusWidget(true, NULL)
{
	mAllowSudo = allowSudo;
	mDescription = description;
	mRequest = request;
	mChannel = channel;
	mPermissionError = false;
	setButtons(Cancel);

	connect(this, SIGNAL(updateRethreadSignal()), this, SLOT(update()), Qt::QueuedConnection);
	connect(this, SIGNAL(buttonClicked(StatusWidget::Button)), this, SLOT(onButtonClicked(StatusWidget::Button)));

	mStatus = Working;
	mRequest->setStatusWidget(this);
}

RequestStatusWidget::~RequestStatusWidget()
{
}

void RequestStatusWidget::success()
{
	mStatus = Success;
	update();
}

void RequestStatusWidget::error(RemoteRequest::Error error)
{
	mErrorMessage = error.message;
	mPermissionError = (error.type == RemoteRequest::ErrPermission);
	mStatus = Error;

	update();
}

void RequestStatusWidget::showEvent(QShowEvent *)
{
	sendRequest();
}

void RequestStatusWidget::sendRequest()
{
	try
	{
		mChannel->sendRequest(mRequest);
	}
	catch (QString error)
	{
		RemoteRequest::Error e;
		e.message = error;
		e.type = RemoteRequest::ErrUnspecified;
		this->error(e);
	}

	update();
}

void RequestStatusWidget::update()
{
	//	Only run on the main thread, if visible.
	if (!Tools::isMainThread())
	{
		emit updateRethreadSignal();
		return;
	}
	if (!isVisible()) return;

	switch (mStatus)
	{
	case Success:
		close(true);
		break;

	case Error:
		setStatus(QPixmap(":/icons/error.png"), tr("Error: %1").arg(mErrorMessage));
		setButtons(Retry | Cancel | (mPermissionError && mAllowSudo ? SudoRetry : None));
		break;

	default:
		setStatus(QPixmap(":/icons/loading.png"), mDescription);
		setButtons(Cancel);
		break;
	}
}

void RequestStatusWidget::onButtonClicked(StatusWidget::Button button)
{
	switch (button)
	{
	case Cancel:
		close(false);
		break;

	case Retry:
		sendRequest();
		break;

	case SudoRetry:
		close(SudoRequestedResult);
		break;

	default:break;
	}
}

