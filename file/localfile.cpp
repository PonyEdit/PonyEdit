#include <QDebug>

#include "localfile.h"

LocalFile::LocalFile(const Location& location) : BaseFile(location)
{
	connect(this, SIGNAL(localFileOpened(QByteArray,bool)), this, SLOT(fileOpened(QByteArray,bool)), Qt::QueuedConnection);
}

BaseFile* LocalFile::newFile(const QByteArray& content)
{
	mContent = content;
	save();
	return this;
}

void LocalFile::open()
{
	QFile fileHandle(mLocation.getPath());
	fileHandle.open(QIODevice::ReadOnly);

	bool readOnly = false;
	if(!(fileHandle.permissions() & QFile::WriteUser))
		readOnly = true;

	QTextStream stream(&fileHandle);

	QString content = stream.readAll();

	fileHandle.close();

	emit localFileOpened(content.toUtf8(), readOnly);
}

void LocalFile::save()
{
	QFile fileHandle(mLocation.getPath());
	fileHandle.open(QIODevice::WriteOnly);

	if(!(fileHandle.permissions() & QFile::WriteUser))
	{
		saveFailed();
		return;
	}

	QTextStream stream(&fileHandle);

	stream << mContent;
	stream.flush();

	fileHandle.close();

	mLastSavedRevision = mRevision;

	setOpenStatus(Ready);
}

void LocalFile::close()
{
	setOpenStatus(Closing);
	BaseFile::closeCompleted();
}
