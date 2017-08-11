#include <QDebug>
#include <QStylePainter>

#include "customtreedelegate.h"
#include "customtreeentry.h"
#include "customtreemodel.h"

CustomTreeDelegate::CustomTreeDelegate( CustomTreeModel* model ) {
	mModel = model;
}

void CustomTreeDelegate::paint( QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index ) const {
	CustomTreeEntry* entry = mModel->getEntry( index );
	entry->setHover( option.state & QStyle::State_MouseOver );

	// Always start with the standard draw. If the item is custom drawn, the model will give a blank label
	// to allow the label to be drawn on afterwards. If not custom drawn, this is the only step needed.
	QStyledItemDelegate::paint( painter, option, index );
	if ( ! entry->isCustomDrawn() ) {
		return;
	}

	entry->customDraw( painter, option );
}
