#ifndef SEARCHRESULTDELEGATE_H
#define SEARCHRESULTDELEGATE_H

HIDE_COMPILE_WARNINGS

#include <QStyledItemDelegate>

UNHIDE_COMPILE_WARNINGS

class SearchResultModel;
class SearchResultDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
	explicit SearchResultDelegate(SearchResultModel* model, QObject *parent = 0);

	SearchResultDelegate(SearchResultDelegate const&) = delete;
	SearchResultDelegate& operator=(SearchResultDelegate const&) = delete;
			
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	SearchResultModel* mModel;
};

#endif // SEARCHRESULTDELEGATE_H
