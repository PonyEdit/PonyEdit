#include "remoterequest.h"

RemoteRequest::RemoteRequest()
{
}

void RemoteRequest::error(const QString& message)
{
	Error e;
	e.message = message;
	e.type = ErrUnspecified;
	error(e);
}
