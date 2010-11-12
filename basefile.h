#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <QByteArray>
#include <QTextDocument>
#include <QPlainTextDocumentLayout>

#include "location.h"
class Editor;

class BaseFile : public QObject
{
	Q_OBJECT

public:
	struct Change { int revision; int position; int remove; QByteArray insert; };
	enum OpenStatus { Closed, Opening, Open, Disconnected, Reconnecting, Error };

	static BaseFile* getFile(const Location& location);
	static const QList<BaseFile*>& getActiveFiles();

	inline const QByteArray& getContent() const { return mContent; }
	inline const Location& getLocation() const { return mLocation; }
	inline QTextDocument* getTextDocument() { return mDocument; }
	inline const QString& getError() const { return mError; }
	inline bool isOpen() const { return mOpenStatus == Open; }
	inline bool isOpening() const { return mOpenStatus == Opening; }
	inline bool isClosed() const { return mOpenStatus == Closed; }
	inline OpenStatus getOpenStatus() const { return mOpenStatus; }

	virtual void open() = 0;
	virtual void save() = 0;
	void openError(const QString& error);
	void savedRevision(int revision);

	inline const QList<Editor*>& getAttachedEditors() { return mAttachedEditors; }
	void editorAttached(Editor* editor);	//	Call only from Editor constructor.
	void editorDetached(Editor* editor);	//	Call only from Editor destructor.

	QString getChecksum() const;

public slots:
	void fileOpened(const QByteArray& content);
	void documentChanged(int position, int removeChars, int added);

signals:
	void fileOpenedRethreadSignal(const QByteArray& content);
	void openStatusChanged(int newStatus);

protected:
	BaseFile(const Location& location);
	virtual ~BaseFile();
	void setOpenStatus(OpenStatus newStatus);

	virtual void handleDocumentChange(int position, int removeChars, const QByteArray& insert);
	virtual bool storeChanges() { return false; }

	Location mLocation;
	QByteArray mContent;
	QString mError;

	QTextDocument *mDocument;
	QPlainTextDocumentLayout *mDocumentLayout;

	bool mChanged;
	bool mDosLineEndings;
	int mRevision;
	int mLastSavedRevision;
	bool mIgnoreChanges;	//	Used to disregard change notifications while setting up the text document initially.

	QList<Change*> mChangesSinceLastSave;
	bool mChangeBufferOversized;
	quint64 mChangeBufferSize;

	OpenStatus mOpenStatus;
	QList<Editor*> mAttachedEditors;

	static QList<BaseFile*> sActiveFiles;
};

#endif // FILE_H
