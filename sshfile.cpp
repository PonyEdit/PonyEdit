#include <QDebug>
#include "sshfile.h"
#include "sshrequest.h"

SshFile::SshFile(SshRemoteController* controller, int bufferId, const Location& location, const QByteArray& data) : File(location, data)
{
	mBufferId = bufferId;
	mController = controller;
}

void SshFile::changeDocument(int position, int removeChars, const QByteArray& insert)
{
	File::changeDocument(position, removeChars, insert);
	qDebug() << "Edit revision " << mRevision;

	mController->sendRequest(new SshRequest_changeBuffer(mBufferId, position, removeChars, insert));
}

void SshFile::savedRevision(int revision)
{
	mLastSavedRevision = revision;
	qDebug() << "Saved revision " << revision;
}

void SshFile::save()
{
	qDebug() << "Saving revision " << mRevision;
	mController->sendRequest(new SshRequest_saveBuffer(mBufferId, this, mRevision, mData));
}
