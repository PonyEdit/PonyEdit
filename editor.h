#ifndef EDITOR_H
#define EDITOR_H

#include <QLabel>
#include <QTextEdit>
#include <QStackedWidget>
#include <QProgressBar>

#include "location.h"
#include "basefile.h"

#include "codeeditor.h"

class Editor : public QStackedWidget
{
    Q_OBJECT
public:
	explicit Editor(BaseFile* file);
	~Editor();
	void save();

	BaseFile* getFile() const { return mFile; }
	const Location& getLocation() const { return mFile->getLocation(); }

public slots:
	void openStatusChanged(int openStatus);
	void fileOpenProgress(int percent);
	void find(const QString& text, bool backwards);

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
