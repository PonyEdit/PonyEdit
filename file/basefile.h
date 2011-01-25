#ifndef FILE_H
#define FILE_H

#include <QObject>
#include <QByteArray>
#include <QString>
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
	struct Change { int revision; int position; int remove; QString insert; };
	enum OpenStatus { Closed, Loading, LoadError, Ready, Disconnected, Reconnecting, Repairing, SyncError, Closing };
	static const char* sStatusLabels[];

	static BaseFile* getFile(const Location& location);
	static const QList<BaseFile*>& getActiveFiles();
	virtual ~BaseFile();

	inline const QString& getContent() const { return mContent; }
	inline const Location& getLocation() const { return mLocation; }
	inline QTextDocument* getTextDocument() { return mDocument; }
	inline const QString& getError() const { return mError; }
	inline bool isClosed() const { return mOpenStatus == Closed; }
	inline OpenStatus getOpenStatus() const { return mOpenStatus; }
	inline int getLoadingPercent() const { return mLoadingPercent; }
	inline bool hasUnsavedChanges() const { return mRevision > mLastSavedRevision; }
	inline bool isReadOnly() const { return mReadOnly; }

	virtual BaseFile* newFile(const QString& content) = 0;
	virtual void open() = 0;
	virtual void save() = 0;
	virtual void close() = 0;	// Warning: This call is asynchronous in some kinds of file; eg SshFile.
	void openError(const QString& error);
	void savedRevision(int revision, const QByteArray& checksum);
	void fileOpenProgressed(int percent);

	void saveFailed();

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
	void fileOpened(const QString& content, bool readOnly);
	void documentChanged(int position, int removeChars, int added);
	void closeCompleted();

signals:
	void fileOpenedRethreadSignal(const QString& content, bool readOnly);
	void closeCompletedRethreadSignal();
	void fileOpenProgress(int percent);
	void openStatusChanged(int newStatus);
	void unsavedStatusChanged();
	void saveFailed(const QString& error);

protected:
	BaseFile(const Location& location);
	void setOpenStatus(OpenStatus newStatus);

	virtual void handleDocumentChange(int position, int removeChars, const QString& insert);
	virtual void setLastSavedRevision(int lastSavedRevision);

	void autodetectSyntax();

	Location mLocation;
	QString mContent;
	QString mError;

	QTextDocument* mDocument;
	QPlainTextDocumentLayout* mDocumentLayout;

	bool mChanged;
	bool mDosLineEndings;
	bool mReadOnly;
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
