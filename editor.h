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

private:
	QTextEdit* mEditor;

	QWidget* mWorkingPane;
	QLabel* mWorkingIcon;
	QLabel* mWorkingText;
};

#endif // EDITOR_H
