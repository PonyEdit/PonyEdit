#ifndef CUSTOMTREEDELEGATE_H
#define CUSTOMTREEDELEGATE_H

HIDE_COMPILE_WARNINGS

#include <QStyledItemDelegate>

UNHIDE_COMPILE_WARNINGS

class CustomTreeModel;
class CustomTreeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
	explicit CustomTreeDelegate(CustomTreeModel* model);

	CustomTreeDelegate(CustomTreeDelegate const&) = delete;
	CustomTreeDelegate& operator=(CustomTreeDelegate const&) = delete;
		
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	CustomTreeModel* mModel;
};

#endif // CUSTOMTREEDELEGATE_H
