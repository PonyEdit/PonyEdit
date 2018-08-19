#include "file/location.h"
#include "filelistdelegate.h"

QIcon FileListDelegate::sUnreadable;

FileListDelegate::FileListDelegate( QObject *parent ) :
	QStyledItemDelegate( parent ) {}

void FileListDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
	QStyledItemDelegate::paint( painter, option, index );

	if ( index.column() == 0 ) {
		auto location = index.data( Qt::UserRole ).value< Location >();
		if ( ! location.canRead() ) {
			if ( sUnreadable.isNull() ) {
				sUnreadable = QIcon( ":/icons/unreadable.png" );
			}

			int offset = option.rect.height() - 8;
			sUnreadable.paint( painter, option.rect.left() + offset, option.rect.top() + offset, 8, 8 );
		}
	}
}
