#ifndef REMOTEREQUEST_H
#define REMOTEREQUEST_H

#include <QString>

class RequestStatusWidget;
class RemoteRequest
{
public:
	enum ErrorType { ErrOk = 0, ErrUnspecified = 0, ErrPermission = 1 };
	struct Error { ErrorType type; QString message; };

    RemoteRequest();

	void setStatusWidget(RequestStatusWidget* statusWidget);

	void handleError(const QString& message);
	void handleError(const Error& err);
	void handleSuccess();

protected:
	virtual void error(const Error& /* err */) {}
	virtual void success() {}

private:
	RequestStatusWidget* mStatusWidget;
};

#endif // REMOTEREQUEST_H
