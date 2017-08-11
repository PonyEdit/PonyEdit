#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>

class LineNumberWidget;
class SyntaxHighlighter;
class BaseFile;
class WindowManager;

class CodeEditor : public QPlainTextEdit {
	Q_OBJECT

	public:
		CodeEditor( BaseFile* file, QWidget *parent = 0 );

		void lineNumberAreaPaintEvent( QPaintEvent *event );
		int lineNumberAreaWidth();

		int firstNonWhiteSpace( const QTextBlock& block );

		void updateFont();

		void zoomOut();
		void zoomIn();

		void deleteLine();

	protected:
		void resizeEvent( QResizeEvent *event );
		void keyPressEvent( QKeyEvent* event );
		void wheelEvent( QWheelEvent *e );
		void applyIndent( QTextCursor& cursor, bool outdent );
		void focusInEvent( QFocusEvent *e );

	public slots:
		void undo();
		void redo();

	private slots:
		void updateLineNumberAreaWidth( int newBlockCount );
		void updateLineNumberArea( const QRect &, int );
		void highlightCurrentLine();
		void highlightMatchingParenthesis();

	private:
		LineNumberWidget *mLineNumberWidget;
		SyntaxHighlighter* mSyntaxHighlighter;
		BaseFile* mFile;

		QTextEdit::ExtraSelection mCurrentLine;
		QList< QTextEdit::ExtraSelection > mMatchingParenthesis;
};

#endif  // CODEEDITOR_H
