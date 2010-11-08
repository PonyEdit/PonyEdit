#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

class LineNumberWidget;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
	CodeEditor(QWidget *parent = 0);

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

protected:
	void resizeEvent(QResizeEvent *event);

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void updateLineNumberArea(const QRect &, int);
	void highlightCurrentLine();

private:
	LineNumberWidget *mLineNumberWidget;

signals:
public slots:

};

#endif // CODEEDITOR_H
