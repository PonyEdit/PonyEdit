#include <QDebug>
#include "sshhost.h"
#include "sshfile.h"
#include "sshrequest.h"
#include "sshremotecontroller.h"

SshFile::SshFile(const Location& location) : BaseFile(location)
{
	mHost = location.getRemoteHost();
	mHost->registerOpenFile(this);
	mBufferId = -1;
}

SshFile::~SshFile()
{
	mHost->unregisterOpenFile(this);
}

void SshFile::open()
{
	if (!mHost->ensureConnection())
		throw(QString("Failed to open file: failed to connect to remote host"));

	mHost->getController()->sendRequest(new SshRequest_open(this));
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

	if (!mHost->ensureConnection())
		throw(QString("Failed to update file: failed to connect to remote host"));

	mHost->getController()->sendRequest(new SshRequest_changeBuffer(mBufferId, position, removeChars, insert));
}

void SshFile::savedRevision(int revision)
{
	mLastSavedRevision = revision;
	qDebug() << "Saved revision " << revision;
}

void SshFile::save()
{
	if (!mHost->ensureConnection())
		throw(QString("Failed to update file: failed to connect to remote host"));

	qDebug() << "Saving revision " << mRevision;
	mHost->getController()->sendRequest(new SshRequest_saveBuffer(mBufferId, this, mRevision, mContent));
}
