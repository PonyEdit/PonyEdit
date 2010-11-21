#ifndef FILELISTITEMDELEGATE_H
#define FILELISTITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QTreeView>

class FileListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
	explicit FileListItemDelegate(QTreeView *parent = 0);
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	QString squashLabel(const QString& label, const QFontMetrics& metrics, int width) const;

	QTreeView* mParent;
};

#endif // FILELISTITEMDELEGATE_H
