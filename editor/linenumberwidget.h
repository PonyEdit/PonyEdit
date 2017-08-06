#ifndef LINENUMBERWIDGET_H
#define LINENUMBERWIDGET_H

HIDE_COMPILE_WARNINGS

#include <QWidget>
#include <QSize>
#include <QPaintEvent>

UNHIDE_COMPILE_WARNINGS

class CodeEditor;

class LineNumberWidget : public QWidget
{
public:
	explicit LineNumberWidget(CodeEditor *editor = 0);

	LineNumberWidget(LineNumberWidget const&) = delete;
	LineNumberWidget& operator=(LineNumberWidget const&) = delete;

	QSize sizeHint() const;

protected:
	void paintEvent(QPaintEvent *event);

private:
	CodeEditor *mEditor;

signals:

public slots:

};

#endif // LINENUMBERWIDGET_H
