#include <QDebug>
#include "sshfile.h"
#include "sshrequest.h"
#include "sshremotecontroller.h"

SshFile::SshFile(SshRemoteController* controller, const Location& location) : BaseFile(location)
{
	mController = controller;
	mBufferId = -1;
	loadContent();
}

void SshFile::loadContent()
{
	mController->sendRequest(new SshRequest_open(this));
}

void SshFile::fileOpened(int bufferId, const QByteArray& content)
{
	mBufferId = bufferId;
	BaseFile::fileOpened(content);
}

void SshFile::handleDocumentChange(int position, int removeChars, const QByteArray& insert)
{
	BaseFile::handleDocumentChange(position, removeChars, insert);
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
	mController->sendRequest(new SshRequest_saveBuffer(mBufferId, this, mRevision, mContent));
}
