#include "searchresultdelegate.h"
#include "searchresultmodel.h"
#include <QPainter>
#include <options/options.h>
#include <QDebug>
#include <QEvent>

SearchResultDelegate::SearchResultDelegate(SearchResultModel* model, QObject *parent) :
    QStyledItemDelegate(parent)
{
	mModel = model;
}

void SearchResultDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyledItemDelegate::paint(painter, option, index);

	SearchResultModel::Result* result = mModel->getResultForIndex(index);
	if (result)
	{
		painter->setFont(Options::EditorFont);

		QPen originalPen = painter->pen();
		QRect drawRect = option.rect;

		drawRect.setLeft(drawRect.left() + option.decorationSize.width() + 5);

		//	Draw the line number on the left, in gray.
		QString lineNumber = QString::number(result->lineNumber + 1);
		painter->setPen(QPen(QColor(172, 172, 172)));
		painter->drawText(drawRect, lineNumber);
		drawRect.setLeft(drawRect.left() + painter->boundingRect(drawRect, lineNumber).width() + 10);

		QString left = result->matchedLine.mid(0, result->start);
		QString match = result->matchedLine.mid(result->start, result->length);

		int leftWidth = painter->boundingRect(drawRect, left).width();
		int matchWidth = painter->boundingRect(drawRect, match).width();

		//	Draw a highlight rectangle
		painter->fillRect(QRect(drawRect.left() + leftWidth, drawRect.top() + 1, matchWidth, drawRect.height() - 2), QColor(255, 255, 0));

		painter->setPen(originalPen);
		painter->drawText(drawRect, result->matchedLine);
	}
}
