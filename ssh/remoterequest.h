#ifndef REMOTEREQUEST_H
#define REMOTEREQUEST_H

#include <QString>

class RemoteRequest
{
public:
    RemoteRequest();

	virtual void error(const QString& /*error*/) {};
	virtual void success() {}
};

#endif // REMOTEREQUEST_H
