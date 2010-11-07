#include <QPainter>
#include <QTextBlock>

#include "codeeditor.h"
#include "linenumberwidget.h"

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
	mLineNumberWidget = new LineNumberWidget(this);

	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));

	updateLineNumberAreaWidth(0);
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

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
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
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();

	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, mLineNumberWidget->width(), fontMetrics().height(),
								Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}
