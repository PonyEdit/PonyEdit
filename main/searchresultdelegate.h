#ifndef SEARCHRESULTDELEGATE_H
#define SEARCHRESULTDELEGATE_H

#include <QStyledItemDelegate>

class SearchResultModel;
class SearchResultDelegate : public QStyledItemDelegate {
	Q_OBJECT

	public:
		explicit SearchResultDelegate( SearchResultModel* model, QObject *parent = 0 );
		void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

	private:
		SearchResultModel* mModel;
};

#endif  // SEARCHRESULTDELEGATE_H
