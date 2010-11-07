#ifndef EDITOR_H
#define EDITOR_H

#include <QLabel>
#include <QTextEdit>
#include <QStackedWidget>

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

private:
	void showLoading();
	void showError(const QString& error);

	CodeEditor* mEditor;
	QTextDocument* mDocument;

	QWidget* mWorkingPane;
	QLabel* mWorkingIcon;
	QLabel* mWorkingText;
	BaseFile* mFile;
};

#endif // EDITOR_H
