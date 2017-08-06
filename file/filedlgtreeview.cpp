#include "filedlgtreeview.h"

FileDlgTreeView::FileDlgTreeView( QObject *parent )
    : QTreeView( parent ) {
}

void FileDlgTreeView::mousePressEvent( QMouseEvent *event ) {
	if ( event->button() == Qt::RightButton )
		return;
	QTreeView::mousePressEvent( event );
}
