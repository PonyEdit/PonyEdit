#include <QDebug.h>

#include "basefile.h"
#include "tools.h"

BaseFile::BaseFile(const Location& location)
{
	mOpenStatus = Loading;
	mLocation = location;

	connect(&mDocument, SIGNAL(contentsChange(int,int,int)), this, SLOT(documentChanged(int,int,int)));
	connect(this, SIGNAL(fileOpenedRethreadSignal(QByteArray)), this, SLOT(fileOpened(QByteArray)), Qt::QueuedConnection);
}

void BaseFile::documentChanged(int position, int removeChars, int charsAdded)
{
	if (mOpenStatus != Ready)
		return;

	QByteArray added = "";
	for (int i = 0; i < charsAdded; i++)
	{
		QChar c = mDocument.characterAt(i + position);
		if (c == QChar::ParagraphSeparator || c == QChar::LineSeparator)
			c = QLatin1Char('\n');
		else if (c == QChar::Nbsp)
			c = QLatin1Char(' ');

		added += c;
	}

	this->handleDocumentChange(position, removeChars, added);
}

void BaseFile::handleDocumentChange(int position, int removeChars, const QByteArray &insert)
{
	mRevision++;
	mContent.replace(position, removeChars, insert);
	mChanged = true;
}

void BaseFile::fileOpened(const QByteArray& content)
{
	//	If this is not the main thread, move to the main thread.
	if (!Tools::isMainThread())
	{
		emit fileOpenedRethreadSignal(content);
		return;
	}

	mContent = content;

	//	Detect line ending mode, then convert it to unix-style. Use unix-style line endings everywhere, only convert to DOS at save time.
	mDosLineEndings = mContent.contains("\r\n");
	if (mDosLineEndings)
		mContent.replace("\r\n", "\n");

	mChanged = false;
	mRevision = 0;
	mLastSavedRevision = 0;

	mDocument.setPlainText(content);

	mOpenStatus = Ready;
	emit openStatusChanged(mOpenStatus);
}

void BaseFile::openError(const QString& error)
{
	mError = error;
	mOpenStatus = Error;
	emit openStatusChanged(mOpenStatus);
}



