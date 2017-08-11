#ifndef FILEDLGTREEDELEGATE_H
#define FILEDLGTREEDELEGATE_H

#include <QStyledItemDelegate>

class FileDlgTreeDelegate : public QStyledItemDelegate {
	Q_OBJECT

	public:
		explicit FileDlgTreeDelegate( QObject *parent = 0 );
		virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
};

#endif  // FILEDLGTREEDELEGATE_H
