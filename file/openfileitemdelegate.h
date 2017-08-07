#ifndef FILELISTITEMDELEGATE_H
#define FILELISTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QTreeView>

#include "openfiletreeview.h"

class OpenFileItemDelegate : public QStyledItemDelegate
{
Q_OBJECT
public:
explicit OpenFileItemDelegate( OpenFileTreeView *parent = 0 );
virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
OpenFileTreeView* mParent;
};

#endif	// FILELISTITEMDELEGATE_H
