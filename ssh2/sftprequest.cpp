#include "sftprequest.h"
#include <QRegExp>
#include <QDebug>

SFTPRequest::SFTPRequest(SFTPRequest::Type type, const Callback& callback) :
    mType(type),
    mPath(),
    mIncludeHidden(),
    mCallback(callback),
    mContent(),
    mRevision(),
    mUndoLength()
{}

void SFTPRequest::setPath(const QString& path)
{
	mPath = path;
	mPath.replace(QRegExp("^~"), ".");
}
