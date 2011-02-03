#ifndef REMOTEREQUEST_H
#define REMOTEREQUEST_H

#include <QString>

class RemoteRequest
{
public:
	enum ErrorType { ErrOk = 0, ErrUnspecified = 0, ErrPermission = 1 };
	struct Error { ErrorType type; QString message; };

    RemoteRequest();

	virtual void error(const Error& /* err */) {}
	void error(const QString& message);
	virtual void success() {}
};

#endif // REMOTEREQUEST_H
