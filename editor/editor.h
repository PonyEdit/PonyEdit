#ifndef EDITOR_H
#define EDITOR_H

#include <QLabel>
#include <QTextEdit>
#include <QStackedWidget>
#include <QProgressBar>

#include "file/location.h"
#include "file/basefile.h"

#include "editor/codeeditor.h"

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

public slots:
	void openStatusChanged(int openStatus);
	void fileOpenProgress(int percent);
	void find(const QString& text, bool backwards);
	void replace(const QString& findText, const QString& replaceText, bool all);
	void setFocus();

private:
	void showLoading();
	void showError(const QString& error);

	bool mFirstOpen;

	BaseFile* mFile;
	CodeEditor* mEditor;
	QTextDocument* mDocument;

	QWidget* mWorkingPane;
	QLabel* mWorkingIcon;
	QLabel* mWorkingText;
	QProgressBar* mProgressBar;
};

#endif // EDITOR_H
