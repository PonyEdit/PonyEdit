#ifndef FILELISTITEMDELEGATE_H
#define FILELISTITEMDELEGATE_H

HIDE_COMPILE_WARNINGS

#include <QStyledItemDelegate>
#include <QTreeView>

UNHIDE_COMPILE_WARNINGS

#include "openfiletreeview.h"

class OpenFileItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
	explicit OpenFileItemDelegate(OpenFileTreeView *parent = 0);

	OpenFileItemDelegate(OpenFileItemDelegate const&) = delete;
	OpenFileItemDelegate& operator=(OpenFileItemDelegate const&) = delete;

	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	OpenFileTreeView* mParent;
};

#endif // FILELISTITEMDELEGATE_H
