#include "sftprequest.h"
#include <QRegExp>
#include <QDebug>

SFTPRequest::SFTPRequest(SFTPRequest::Type type, const Callback& callback)
	: mType(type), mCallback(callback)
{
}

void SFTPRequest::setPath(const QString& path)
{
	mPath = path;
	mPath.replace(QRegExp("^~"), ".");
}
