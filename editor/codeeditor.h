#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QTextCursor>
#include <QTextBlock>

class LineNumberWidget;
class SyntaxHighlighter;
class BaseFile;

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
	CodeEditor(BaseFile* file, QWidget *parent = 0);

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

	int firstNonWhiteSpace(const QTextBlock& block);

	void updateFont();

	void zoomOut();
	void zoomIn();

protected:
	void resizeEvent(QResizeEvent *event);
	void keyPressEvent(QKeyEvent* event);
	void wheelEvent(QWheelEvent *e);
	void applyIndent(QTextCursor& cursor, bool outdent);

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void updateLineNumberArea(const QRect &, int);
	void highlightCurrentLine();

private:
	LineNumberWidget *mLineNumberWidget;
	SyntaxHighlighter* mSyntaxHighlighter;
	BaseFile* mFile;
};

#endif // CODEEDITOR_H
