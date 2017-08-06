#include "filedlgtreedelegate.h"
#include <QApplication>
#include <QDebug>
#include <QIcon>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>

FileDlgTreeDelegate::FileDlgTreeDelegate( QObject *parent )
    : QStyledItemDelegate( parent ) {
}

void FileDlgTreeDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
	QString label = index.data( Qt::UserRole ).toString();

	//	Always paint the default background
	QStyledItemDelegate::paint( painter, option, index );

	//	Now draw the label on top of it.
	QTextOption o;
	o.setAlignment( Qt::AlignBottom | Qt::AlignLeft );
	sp.drawText( labelRect, label, o );
}
