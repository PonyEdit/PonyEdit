#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <QByteArray>
#include <QTextDocument>
#include <QPlainTextDocumentLayout>

#include "location.h"

class Editor;
class SyntaxHighlighter;
class SyntaxDefinition;

class BaseFile : public QObject
{
	Q_OBJECT

public:
	struct Change { int revision; int position; int remove; QByteArray insert; };
	enum OpenStatus { Closed, Loading, LoadError, Ready, Disconnected, Reconnecting, Repairing, SyncError, Closing };
	static const char* sStatusLabels[];

	static BaseFile* getFile(const Location& location);
	static const QList<BaseFile*>& getActiveFiles();
	virtual ~BaseFile();

	inline const QByteArray& getContent() const { return mContent; }
	inline const Location& getLocation() const { return mLocation; }
	inline QTextDocument* getTextDocument() { return mDocument; }
	inline const QString& getError() const { return mError; }
	inline bool isClosed() const { return mOpenStatus == Closed; }
	inline OpenStatus getOpenStatus() const { return mOpenStatus; }
	inline int getLoadingPercent() const { return mLoadingPercent; }
	inline bool hasUnsavedChanges() const { return mRevision > mLastSavedRevision; }

	virtual void newFile() = 0;
	virtual void open() = 0;
	virtual void save() = 0;
	virtual void close() = 0;	// Warning: This call is asynchronous in some kinds of file; eg SshFile.
	void openError(const QString& error);
	void savedRevision(int revision, const QByteArray& checksum);
	void fileOpenProgressed(int percent);

	inline const QList<Editor*>& getAttachedEditors() { return mAttachedEditors; }
	void editorAttached(Editor* editor);	//	Call only from Editor constructor.
	void editorDetached(Editor* editor);	//	Call only from Editor destructor.

	QString getChecksum() const;
	const Location& getDirectory() const;

	void ignoreChanges() { mIgnoreChanges++; }
	void unignoreChanges() { mIgnoreChanges--; if (mIgnoreChanges < 0) mIgnoreChanges = 0; }

	QString getSyntax() const;
	void setSyntax(const QString& syntaxName);
	void setSyntax(SyntaxDefinition* syntaxDef);

public slots:
	void fileOpened(const QByteArray& content);
	void documentChanged(int position, int removeChars, int added);
	void closeCompleted();

signals:
	void fileOpenedRethreadSignal(const QByteArray& content);
	void closeCompletedRethreadSignal();
	void fileOpenProgress(int percent);
	void openStatusChanged(int newStatus);
	void unsavedStatusChanged();

protected:
	BaseFile(const Location& location);
	void setOpenStatus(OpenStatus newStatus);

	virtual void handleDocumentChange(int position, int removeChars, const QByteArray& insert);
	virtual void setLastSavedRevision(int lastSavedRevision);

	void autodetectSyntax();

	Location mLocation;
	QByteArray mContent;
	QString mError;

	QTextDocument* mDocument;
	QPlainTextDocumentLayout* mDocumentLayout;

	bool mChanged;
	bool mDosLineEndings;
	int mRevision;
	int mIgnoreChanges;	//	To disregard change signals while changing content of QTextDocument programmatically.

	int mLastSavedRevision;
	QByteArray mLastSaveChecksum;

	int mLoadingPercent;
	OpenStatus mOpenStatus;
	QList<Editor*> mAttachedEditors;

	SyntaxHighlighter* mHighlighter;
};

#endif // FILE_H
