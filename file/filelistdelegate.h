#ifndef FILELISTDELEGATE_H
#define FILELISTDELEGATE_H

#include <QStyledItemDelegate>
class FileListDelegate : public QStyledItemDelegate {
public:
	FileListDelegate( QObject *parent );
	virtual void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
	static QIcon sUnreadable;
};

#endif // FILELISTDELEGATE_H
