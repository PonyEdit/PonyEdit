#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <QByteArray>
#include <QTextDocument>

#include "location.h"

class BaseFile : public QObject
{
	Q_OBJECT

public:
	enum OpenStatus { Loading, Ready, Error };

	BaseFile(const Location& location);

	inline const QByteArray& getContent() const { return mContent; }
	inline const Location& getLocation() const { return mLocation; }
	inline QTextDocument* getTextDocument() { return &mDocument; }
	inline const QString& getError() const { return mError; }

	virtual void save() = 0;
	void openError(const QString& error);

public slots:
	void fileOpened(const QByteArray& content);
	void documentChanged(int position, int removeChars, int added);

signals:
	void fileOpenedRethreadSignal(const QByteArray& content);
	void openStatusChanged(int newStatus);

protected:
	virtual void handleDocumentChange(int position, int removeChars, const QByteArray& insert);

	Location mLocation;
	QByteArray mContent;
	QString mError;

	QTextDocument mDocument;

	bool mChanged;
	bool mDosLineEndings;
	int mRevision;
	int mLastSavedRevision;

private:
	OpenStatus mOpenStatus;
};

#endif // FILE_H
