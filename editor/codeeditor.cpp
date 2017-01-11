HIDE_COMPILE_WARNINGS

#include <QPainter>
#include <QTextBlock>
#include <QChar>

UNHIDE_COMPILE_WARNINGS

#include "editor/codeeditor.h"
#include "editor/linenumberwidget.h"
#include "syntax/syntaxhighlighter.h"
#include "options/options.h"
#include "main/globaldispatcher.h"
#include "main/windowmanager.h"
#include "file/basefile.h"
#include "main/editorpanel.h"

CodeEditor::CodeEditor(BaseFile* file, QWidget *parent) : QPlainTextEdit(parent)
	mFile(file),
	mLineNumberWidget(new LineNumberWidget(this))
{
	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightMatchingParenthesis()));

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10)
	{
		max /= 10;
		++digits;
	}

	int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

	return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /*newBlockCount*/)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
	if (dy)
		mLineNumberWidget->scroll(0, dy);
	else
		mLineNumberWidget->update(0, rect.y(), mLineNumberWidget->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	mLineNumberWidget->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(mLineNumberWidget);
	painter.fillRect(event->rect(), Qt::lightGray);

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();

	qreal offset = Options::EditorFontZoom / 100.0;

	qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
	qreal bottom = blockBoundingGeometry(block).translated(contentOffset()).bottom();

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom <= frameRect().height()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::darkGray);
			painter.drawText(0, (int)(top + offset), mLineNumberWidget->width(), fontMetrics().height(),
								Qt::AlignRight, number);
		}

		block = block.next();
		top = blockBoundingGeometry(block).translated(contentOffset()).top();
		bottom = blockBoundingRect(block).translated(contentOffset()).bottom();
		++blockNumber;
	}
}

void CodeEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;
	extraSelections.append(mMatchingParenthesis);

	if (!isReadOnly()) {
		QColor lineColor = QColor(220, 230, 255);

		mCurrentLine.format.setBackground(lineColor);
		mCurrentLine.format.setProperty(QTextFormat::FullWidthSelection, true);
		mCurrentLine.cursor = textCursor();
		mCurrentLine.cursor.clearSelection();
		extraSelections.append(mCurrentLine);
	}

	setExtraSelections(extraSelections);
}

void CodeEditor::applyIndent(QTextCursor& cursor, bool outdent)
{
	QTextDocument* doc = document();
	int position = cursor.position();
	int column = 0;
	int lastTabPosition = 0;

	for (int i = cursor.block().position(); i < position; i++)
	{
		if (column % Options::TabStopWidth == 0)
			lastTabPosition = i;

		if (doc->characterAt(i) == '\t')
		{
			column -= (column % Options::TabStopWidth);
			column += Options::TabStopWidth;
		}
		else
			column++;
	}

	if (outdent)
	{
		if (column == 0) return;
		int removeSpaces = position - lastTabPosition;

		while (removeSpaces)
		{
			QChar nextChar = doc->characterAt(position - 1);
			if (!nextChar.isSpace())
				break;
			else
			{
				cursor.deletePreviousChar();
				//if (nextChar == '\t')
					//break;
				removeSpaces--;
				position--;
			}
		}
	}
	else
	{
		//	Insert a tab or a set of spaces
		if (Options::IndentSpaces)
		{
			int spacesToInsert = Options::TabStopWidth - (column % Options::TabStopWidth);
			for (int i = 0; i < spacesToInsert; i++)
				cursor.insertText(" ");
		}
		else
			cursor.insertText("\t");
	}
}

int CodeEditor::firstNonWhiteSpace(const QTextBlock& block)
{
	QTextDocument* doc = document();
	int scan = block.position();
	int length = block.length();
	int end = scan + length;

	while (scan < end && doc->characterAt(scan).isSpace())
		scan++;

	if (scan == end)
		scan--;

	return scan;
}

void CodeEditor::undo() {
	mFile->beginUndoBlock();
	QPlainTextEdit::undo();
	mFile->endUndoBlock();
}

void CodeEditor::redo() {
	mFile->beginRedoBlock();
	QPlainTextEdit::redo();
	mFile->endRedoBlock();
}

void CodeEditor::keyPressEvent(QKeyEvent* event)
{
	//	Take special note of undo/redo key chords, for version tracking
	bool isUndo = event->matches(QKeySequence::Undo);
	bool isRedo = event->matches(QKeySequence::Redo);
	if (isUndo)
		mFile->beginUndoBlock();
	else if (isRedo)
		mFile->beginRedoBlock();

	//	Deal with TABs; special case for indenting stuff.
	if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab)
	{
		bool outdent = event->modifiers() & Qt::ShiftModifier;
		textCursor().beginEditBlock();

		//	If stuff is selected, indent each line
		if (textCursor().hasSelection())
		{
			QTextBlock block = document()->findBlock(textCursor().selectionStart());
			QTextCursor tmpCursor = QTextCursor(document());
			tmpCursor.setPosition(firstNonWhiteSpace(block));

			int selectionEnd = textCursor().selectionEnd();
			QTextBlock lastBlock = document()->findBlock(selectionEnd);
			int lastLine = lastBlock.blockNumber();
			if (selectionEnd > lastBlock.position() && lastLine < lastBlock.blockNumber())
				lastLine++;

			bool hasNextBlock = true;
			while(hasNextBlock && tmpCursor.blockNumber() <= lastLine)
			{
				applyIndent(tmpCursor, outdent);

				tmpCursor.movePosition(QTextCursor::StartOfLine);
				hasNextBlock = tmpCursor.movePosition(QTextCursor::NextBlock);
				tmpCursor.setPosition(firstNonWhiteSpace(tmpCursor.block()));
			}
		}
		//	Else indent at the current spot
		else
		{
			QTextCursor tc = textCursor();
			applyIndent(tc, outdent);
			setTextCursor(tc);
		}

		textCursor().endEditBlock();
		event->accept();
		return;
	}

	//	Handle normal keypress
	QPlainTextEdit::keyPressEvent(event);

	if (isUndo)
		mFile->endUndoBlock();
	else if (isRedo)
		mFile->endRedoBlock();

	//	Keep indent on next line, if options say to
	if (Options::IndentMode == Options::KeepIndentOnNextLine)
	{
		if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
		{
			const QTextBlock& currentBlock = textCursor().block();
			const QTextBlock& previousBlock = currentBlock.previous();

			if (previousBlock.isValid())
			{
				QString previousLine = previousBlock.text();
				QString whitespace;
				for (int i = 0; i < previousLine.length() && previousLine.at(i).isSpace(); i++)
					whitespace += previousLine.at(i);
				textCursor().insertText(whitespace);
			}
		}
	}
}

void CodeEditor::wheelEvent(QWheelEvent *e)
{
	if(e->modifiers() == Qt::ControlModifier)
	{
		e->accept();
		if(e->delta() > 0)
			zoomIn();
		else
			zoomOut();
	}
	else
		QPlainTextEdit::wheelEvent(e);
}

void CodeEditor::updateFont()
{
	QFont* font = new QFont(*Options::EditorFont);
	font->setWeight(QFont::Normal);

	int pointSize = font->pointSize();
	pointSize = (pointSize * Options::EditorFontZoom) / 100;
	font->setPointSize(pointSize);

	setFont(*font);
}

void CodeEditor::zoomIn()
{
	Options::EditorFontZoom += 10;

	gDispatcher->emitOptionsChanged();
}

void CodeEditor::zoomOut()
{
	Options::EditorFontZoom -= 10;
	if(Options::EditorFontZoom < 10)
		Options::EditorFontZoom = 10;

	gDispatcher->emitOptionsChanged();
}

void CodeEditor::deleteLine()
{
	QTextCursor cursor(document());

	cursor.setPosition(textCursor().position());

	cursor.beginEditBlock();

	if(cursor.positionInBlock() != 0)
		cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);

	cursor.deletePreviousChar();

	cursor.endEditBlock();
}

void CodeEditor::focusInEvent(QFocusEvent *e)
{
	//	Find the EditorStack that owns me...
	QObject* object;
	for (object = this; object != NULL; object = object->parent())
		if (strcmp("EditorPanel", object->metaObject()->className()) == 0)
			break;

	if ( ! object )
		return;

	EditorPanel* editorPanel = (EditorPanel*)object;
	editorPanel->takeFocus();

	QPlainTextEdit::focusInEvent(e);
}

void CodeEditor::highlightMatchingParenthesis()
{
	QString doc = this->toPlainText();
	int pos = this->textCursor().position();

	QChar leftChar, rightChar, curChar, compareChar, skipChar;
	int leftPos = -1, rightPos = -1, bracketCount = 0;

	if (pos > 0)
		leftChar = doc.at(pos-1);
	if (pos < doc.size())
		rightChar = doc.at(pos);

	if (leftChar == ']' || leftChar == '}' || leftChar == ')') {
		if (leftChar == ']')
			compareChar = '[';
		else if (leftChar == '}')
			compareChar = '{';
		else if (leftChar == ')')
			compareChar = '(';

		rightPos = pos - 1;
		for(int ii = rightPos - 1; ii >= 0; ii--) {
			curChar = doc.at(ii);

			if (!skipChar.isNull()) {
				if(curChar == skipChar)
					skipChar = '\0';

				continue;
			}

			if (bracketCount == 0 && curChar == compareChar) {
				leftPos = ii;
				break;
			}

			if(curChar == '"' || curChar == '\'')
				skipChar = curChar;

			if (curChar == leftChar)
				bracketCount++;

			if (curChar == compareChar)
				bracketCount--;
		}
	}
	else if (rightChar == '[' || rightChar == '{' || rightChar == '(') {
		if (rightChar == '[')
			compareChar = ']';
		else if (rightChar == '{')
			compareChar = '}';
		else if (rightChar == '(')
			compareChar = ')';

		leftPos = pos;
		for(int ii = leftPos + 1; ii < doc.size(); ii++) {
			curChar = doc.at(ii);

			if (!skipChar.isNull()) {
				if(curChar == skipChar)
					skipChar = '\0';

				continue;
			}

			if(bracketCount == 0 && curChar == compareChar) {
				rightPos = ii;
				break;
			}

			if(curChar == '"' || curChar == '\'')
				skipChar = curChar;

			if (curChar == rightChar)
				bracketCount++;

			if (curChar == compareChar)
				bracketCount--;
		}
	}

	QList<QTextEdit::ExtraSelection> extraSelections;
	extraSelections.append(mCurrentLine);
	mMatchingParenthesis.clear();

	if (leftPos < 0 || rightPos < 0) {
		setExtraSelections(extraSelections);
		return;
	}

	QTextEdit::ExtraSelection selection;

	QColor textColor = QColor(255, 0, 0);

	selection.format.setForeground(textColor);
	selection.format.setProperty(QTextFormat::CharFormat, true);

	selection.cursor = textCursor();
	selection.cursor.clearSelection();

	selection.cursor.setPosition(leftPos);
	selection.cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor );
	mMatchingParenthesis.append(selection);

	selection.cursor.setPosition(rightPos);
	selection.cursor.movePosition( QTextCursor::NextCharacter, QTextCursor::KeepAnchor );
	mMatchingParenthesis.append(selection);

	extraSelections.append(mMatchingParenthesis);

	setExtraSelections(extraSelections);

}

