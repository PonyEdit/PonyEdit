#ifndef FILELISTITEMDELEGATE_H
#define FILELISTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QTreeView>

class OpenFileItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
	explicit OpenFileItemDelegate(QTreeView *parent = 0);
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	QTreeView* mParent;
};

#endif // FILELISTITEMDELEGATE_H
