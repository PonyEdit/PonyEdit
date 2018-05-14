#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>

#include "file/basefile.h"
#include "file/openfileitemdelegate.h"
#include "file/openfiletreemodel.h"
#include "main/tools.h"

OpenFileItemDelegate::OpenFileItemDelegate( OpenFileTreeView *parent ) :
	QStyledItemDelegate( parent ) {
	mParent = parent;
}

void OpenFileItemDelegate::paint( QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index ) const {
	Location location = index.data( OpenFileTreeModel::LocationRole ).value< Location >();
	BaseFile *file = reinterpret_cast< BaseFile * >( index.data( OpenFileTreeModel::FileRole ).value< void * >() );
	OpenFileTreeModel::Level level = static_cast< OpenFileTreeModel::Level >( index.data( OpenFileTreeModel::TypeRole ).toInt() );
	QString label = index.data( OpenFileTreeModel::LabelRole ).toString();

	// Always paint the default background
	QStyledItemDelegate::paint( painter, option, index );

	if ( mParent->closeColumn() && index.column() == mParent->closeColumn() ) {
		// Show the close icon, but it's only visible on mouse-over.
		QStyledItemDelegate::paint( painter, option, index );
		if ( option.state & QStyle::State_MouseOver ) {
			painter->drawPixmap( option.rect.left(),
			                     option.rect.top(),
			                     16,
			                     16,
			                     QPixmap( ":/icons/cross.png" ) );
		}
	} else if ( mParent->refreshColumn() && index.column() == mParent->refreshColumn() ) {
		// Show the refresh icon, but it's only visible on mouse-over.
		QStyledItemDelegate::paint( painter, option, index );
		if ( option.state & QStyle::State_MouseOver ) {
			painter->drawPixmap( option.rect.left(),
			                     option.rect.top(),
			                     16,
			                     16,
			                     QPixmap( ":/icons/resync.png" ) );
		}
	} else {
		// Column 0 contains the label, and if it's a file also the status.
		QRect labelRect = option.rect;

		QPaintDevice *device = painter->device();
		painter->end();

		{
			QStylePainter sp( device, mParent );

			if ( level == OpenFileTreeModel::File ) {
				BaseFile::OpenStatus fileStatus = file->getOpenStatus();

				if ( file->hasUnsavedChanges() ) {
					sp.drawPixmap( labelRect.right() - 16,
					               labelRect.top(),
					               16,
					               16,
					               QPixmap( ":/icons/filechanged.png" ) );
					labelRect.adjust( 0, 0, -18, 0 );
				}

				if ( fileStatus == BaseFile::Disconnected || fileStatus == BaseFile::Reconnecting ) {
					sp.drawPixmap( labelRect.right() - 16,
					               labelRect.top(),
					               16,
					               16,
					               QPixmap( ":/icons/disconnected.png" ) );
					labelRect.adjust( 0, 0, -18, 0 );
				} else if ( fileStatus == BaseFile::Repairing ) {
					sp.drawPixmap( labelRect.right() - 16,
					               labelRect.top(),
					               16,
					               16,
					               QPixmap( ":/icons/resync.png" ) );
					labelRect.adjust( 0, 0, -18, 0 );
				}

				if ( file->getProgress() > -1 ) {
					// Show a progress bar while loading the file...
					QStyleOptionProgressBar so;
					#ifdef Q_OS_MAC
					int topOffset = 0;
					#else
					int topOffset = 2;
					#endif
					so.rect = QRect( labelRect.right() - 32,
					                 labelRect.top() + topOffset,
					                 32,
					                 labelRect.height() - 4 );
					so.direction = Qt::LeftToRight;
					so.minimum = 0;
					so.maximum = 100;
					so.progress = file->getProgress();

					sp.drawControl( QStyle::CE_ProgressBar, so );

					labelRect.adjust( 0, 0, -34, 0 );
				}

				// Draw the icon
				location.getIcon().paint( &sp, labelRect.left(), labelRect.top(), 16, 16 );
				labelRect.adjust( 16, 0, 0, 0 );
			}

			// Add some padding around the text...
			labelRect.adjust( 2, 0, -2, 0 );

			// Actually draw the text
			QFontMetrics fontMetrics = option.fontMetrics;
			if ( level == OpenFileTreeModel::Host ) {
				sp.setPen( location.isSudo() ? QColor( 0xCE4848 ) : QColor( 0x6490C1 ) );
				QFont f = option.font;
				f.setPointSize( f.pointSize() - 2 );
				f.setItalic( true );
				f.setBold( true );
				sp.setFont( f );
				labelRect.adjust( 0, 0, 0, -3 );
				fontMetrics = QFontMetrics( f );

				sp.drawLine( option.rect.left(),
				             option.rect.bottom() - 2,
				             option.rect.right(),
				             option.rect.bottom() - 2 );
			} else {
				sp.setFont( option.font );
			}

			if ( level == OpenFileTreeModel::File ) {
				label = option.fontMetrics.elidedText( label, Qt::ElideMiddle, labelRect.width() );
			} else {
				label = Tools::squashLabel( label, fontMetrics, labelRect.width() );
			}

			QTextOption o;
			o.setAlignment( Qt::AlignBottom | Qt::AlignLeft );
			sp.drawText( labelRect, label, o );
		}

		painter->begin( device );
	}
}
