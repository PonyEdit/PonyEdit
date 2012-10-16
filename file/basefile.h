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
	enum OpenStatus { Loading, LoadError, Ready, /**/Disconnected, Reconnecting, Repairing, SyncError, /**/Closing, Closed };
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
	inline int getProgress() const { return mProgress; }
	inline bool hasUnsavedChanges() const { return mChanged; }
	inline bool isReadOnly() const { return mReadOnly; }

	virtual BaseFile* newFile(const QString& content) = 0;
	virtual void open() = 0;
	virtual void save() = 0;
	virtual void close() = 0;	// Warning: This call is asynchronous in some kinds of file; eg SlaveFile.
	virtual void refresh() = 0;	// Warning: This call is asynchronous in some kinds of file; eg SlaveFile.
	virtual bool canClose() { return true; }
	void savedRevision(int revision, int undoLength, const QByteArray& checksum);

	inline const QList<Editor*>& getAttachedEditors() { return mAttachedEditors; }
	void editorAttached(Editor* editor);	//	Call only from Editor constructor.
	void editorDetached(Editor* editor);	//	Call only from Editor destructor.

	QString getChecksum() const;
	static QString getChecksum(const QByteArray& content);
	const Location& getDirectory() const;

	void ignoreChanges() { mIgnoreChanges++; }
	void unignoreChanges() { mIgnoreChanges--; if (mIgnoreChanges < 0) mIgnoreChanges = 0; }

	QString getSyntax() const;
	void setSyntax(const QString& syntaxName);
	void setSyntax(SyntaxDefinition* syntaxDef);

	void beginRedoBlock();	// Note the start & end of undo/redo actions for revision tracking
	void beginUndoBlock();
	void endUndoBlock();
	void endRedoBlock();

	virtual void sudo();

public slots:
	void openSuccess(const QString& content, const QByteArray& checksum, bool readOnly);
	void openFailure(const QString& error, int errorFlags);

	void documentChanged(int position, int removeChars, int added);
	void closeCompleted();
	void saveFailure(const QString& errorMessage, bool permissionError);

signals:
	void fileOpenedRethreadSignal(const QString& content, const QByteArray& checksum, bool readOnly);
	void closeCompletedRethreadSignal();
	void saveFailedRethreadSignal(const QString& errorMessage, bool permissionError);
	void fileProgress(int percent);
	void openStatusChanged(int newStatus);
	void unsavedStatusChanged();

protected:
	BaseFile(const Location& location);
	void setOpenStatus(OpenStatus newStatus);
	void setProgress(int percent);

	virtual void handleDocumentChange(int position, int removeChars, const QString& insert);
	virtual void setLastSavedRevision(int lastSavedRevision);

	virtual void reconnect() {}		//	Called when connection dropouts have recovered or relocating a file

	void autodetectSyntax();

	Location mLocation;
	QString mContent;
	QString mError;

	QTextDocument* mDocument;
	QPlainTextDocumentLayout* mDocumentLayout;

	bool mChanged;
	bool mDosLineEndings;
	bool mReadOnly;
	int mIgnoreChanges;	//	To disregard change signals while changing content of QTextDocument programmatically.
	int mInUndoBlock;
	int mInRedoBlock;


	int mRevision;	//	Revision number is pure changes for pumping change queues; undos increase revision number!!
	int mLastSavedRevision;
	int mLastSavedUndoLength;	//	Undo length is used only for detecting unsaved changes; it helps work out if the user has undone all unsaved changes
	QByteArray mLastSaveChecksum;

	int mProgress;
	OpenStatus mOpenStatus;
	QList<Editor*> mAttachedEditors;

	SyntaxHighlighter* mHighlighter;
};

#endif // FILE_H
