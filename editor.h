#ifndef EDITOR_H
#define EDITOR_H

#include "location.h"
#include <QLabel>
#include <QTextEdit>
#include <QStackedWidget>

class Editor : public QStackedWidget
{
    Q_OBJECT
public:
	explicit Editor(const Location& fileLocation);
	void save();

	File* getFile() const { return mFile; }
	const Location& getLocation() const { return mFileLocation; }

public slots:
	void openFileFailed(const QString& error);
	void openFileSuccessful(File* file);
	void docChanged(int position, int charsRemoved, int charsAdded);

private:
	QTextEdit* mEditor;
	QTextDocument* mDocument;
	Location mFileLocation;

	QWidget* mWorkingPane;
	QLabel* mWorkingIcon;
	QLabel* mWorkingText;
	File* mFile;
};

#endif // EDITOR_H
