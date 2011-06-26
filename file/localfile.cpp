#include <QDebug>

#include "localfile.h"

LocalFile::LocalFile(const Location& location) : BaseFile(location)
{
	connect(this, SIGNAL(localFileOpened(QString,bool)), this, SLOT(fileOpened(QString,bool)), Qt::QueuedConnection);
}

BaseFile* LocalFile::newFile(const QString& content)
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

	emit localFileOpened(content, readOnly);
}

void LocalFile::save()
{
	QFile fileHandle(mLocation.getPath());
	fileHandle.open(QIODevice::WriteOnly);

	if(!(fileHandle.permissions() & QFile::WriteUser))
	{
		saveFailed(tr("Permission denied!"), true);
		return;
	}

	QTextStream stream(&fileHandle);

	stream << mContent;
	stream.flush();

	fileHandle.close();

	savedRevision(mRevision, mDocument->availableUndoSteps(), QByteArray(getChecksum().toAscii()));
}

void LocalFile::close()
{
	setOpenStatus(Closing);
	BaseFile::closeCompleted();
}

void LocalFile::refresh()
{
	open();
}
