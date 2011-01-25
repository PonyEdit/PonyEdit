#ifndef EDITOR_H
#define EDITOR_H

#include <QLabel>
#include <QTextEdit>
#include <QStackedWidget>
#include <QProgressBar>
#include <QVBoxLayout>

#include "file/location.h"
#include "file/basefile.h"
#include "main/mainwindow.h"
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
	void gotoEnd();

	void setReadOnly(bool readOnly);
	void showReadOnlyWarning();

public slots:
	void openStatusChanged(int openStatus);
	void fileOpenProgress(int percent);
	int find(const QString& text, bool backwards, bool caseSensitive, bool useRegex, bool loop = true);
	int replace(const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegex, bool all);
	void setFocus();
	void applyOptions();
	void enableEditing() { setReadOnly(false); }
	void undo() { mEditor->undo(); }
	void redo() { mEditor->redo(); }
	void copy() { mEditor->copy(); }
	void cut() { mEditor->cut(); }
	void paste() { mEditor->paste(); }
	void selectAll() { mEditor->selectAll(); }

private:
	void showLoading();
	void showError(const QString& error);

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
