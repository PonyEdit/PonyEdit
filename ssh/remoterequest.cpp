#include "remoterequest.h"
#include "requeststatuswidget.h"

RemoteRequest::RemoteRequest()
{
	mStatusWidget = NULL;
}

void RemoteRequest::handleError(const QString& message)
{
	Error e;
	e.message = message;
	e.type = ErrUnspecified;
	handleError(e);
}

void RemoteRequest::handleError(const Error &err)
{
	if (mStatusWidget)
		mStatusWidget->error(err);
	else
		error(err);
}

void RemoteRequest::handleSuccess()
{
	if (mStatusWidget)
		mStatusWidget->success();
	else
		success();
}

void RemoteRequest::setStatusWidget(RequestStatusWidget* statusWidget)
{
	mStatusWidget = statusWidget;
}
