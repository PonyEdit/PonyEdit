#ifndef EDITOR_H
#define EDITOR_H

#include <QLabel>
#include <QTextEdit>
#include <QStackedWidget>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QtPrintSupport/QPrinter>

#include "file/location.h"
#include "file/basefile.h"
#include "editor/codeeditor.h"

class EditorWarningBar;
class Editor : public QStackedWidget
{
    Q_OBJECT
public:
	explicit Editor(BaseFile* file);
	~Editor();
	void save();
	void close();

	BaseFile* getFile() const { return mFile; }
	const Location& getLocation() const { return mFile->getLocation(); }
	void fileClosed();				//	Called when the file this editor is attached to, is closed.

	void gotoLine(int lineNumber);
	int currentLine() const { return mEditor->textCursor().blockNumber(); }
	void gotoEnd();

	void setReadOnly(bool readOnly);
	void showReadOnlyWarning();

	inline void print(QPrinter *printer) { mEditor->print(printer); }

	CodeEditor* getCodeEditor() { return mEditor; }

	static QTextCursor find(QTextDocument* doc, const QTextCursor& start, const QString& text, bool backwards, bool caseSensitive, bool useRegExp, bool loop);
	void selectText(int lineNumber, int start, int length);

public slots:
	void openStatusChanged(int openStatus);
	void fileOpenProgress(int percent);
	bool find(const QString& text, bool backwards, bool caseSensitive, bool useRegexp, bool loop = true);
	int replace(const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegex, bool all);
	void setFocus();
	bool hasFocus();
	void applyOptions();
	void enableEditing() { setReadOnly(false); }
	void undo() { mFile->beginUndoBlock(); mEditor->undo(); mFile->endUndoBlock(); }
	void redo() { mFile->beginRedoBlock(); mEditor->redo(); mFile->endRedoBlock(); }
	void copy() { mEditor->copy(); }
	void cut() { mEditor->cut(); }
	void paste() { mEditor->paste(); }
	void selectAll() { mEditor->selectAll(); }
	void deleteLine() { mEditor->deleteLine(); }
	void sudo();

private:
	void showLoading();
	void showError(const QString& error);
	QTextCursor internalFind(const QString& text, bool backwards, bool caseSensitive, bool useRegexp, bool loop = true);

	bool mFirstOpen;

	QWidget* mEditorPane;
	QVBoxLayout* mEditorPaneLayout;
	BaseFile* mFile;
	CodeEditor* mEditor;
	QTextDocument* mDocument;

	QWidget* mWorkingPane;
	QLabel* mWorkingIcon;
	QLabel* mWorkingText;
	QProgressBar* mProgressBar;

	EditorWarningBar* mReadOnlyWarning;
};

#endif // EDITOR_H
