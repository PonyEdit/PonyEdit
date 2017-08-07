#ifndef CUSTOMTREEDELEGATE_H
#define CUSTOMTREEDELEGATE_H

#include <QStyledItemDelegate>

class CustomTreeModel;
class CustomTreeDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
explicit CustomTreeDelegate( CustomTreeModel* model );
void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
CustomTreeModel* mModel;
};

#endif	// CUSTOMTREEDELEGATE_H
