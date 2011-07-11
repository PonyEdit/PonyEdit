#include "searchresultdelegate.h"
#include "searchresultmodel.h"
#include <QPainter>
#include <options/options.h>
#include <QDebug>

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
		QFontMetrics metrics = painter->fontMetrics();

		//	Draw the line number on the left, in gray.
		QString lineNumber = QString::number(result->lineNumber);
		painter->setPen(QPen(QColor(172, 172, 172)));
		painter->drawText(drawRect, lineNumber);
		drawRect.setLeft(drawRect.left() + metrics.width(lineNumber) + 10);

		QString left = result->matchedLine.mid(0, result->start - 1);
		QString match = result->matchedLine.mid(result->start - 1, result->length);
		QString right = result->matchedLine.mid(result->start - 1 + result->length);

		qDebug() << result->start << result->length << match << metrics.width(match);

		painter->setPen(originalPen);
		painter->drawText(drawRect, left);
		drawRect.setLeft(drawRect.left() + metrics.width(left));

		int matchWidth = metrics.width(match);
		painter->fillRect(QRect(drawRect.left(), drawRect.top(), matchWidth, drawRect.height()), QColor(255, 255, 0));
		painter->setPen(QPen(QColor(0, 0, 0)));
		painter->drawText(drawRect, match);
		drawRect.setLeft(drawRect.left() + matchWidth);

		painter->setPen(originalPen);
		painter->drawText(drawRect, right);
		drawRect.setLeft(drawRect.left() + metrics.width(right));
	}
}
