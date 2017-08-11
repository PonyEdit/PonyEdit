#ifndef DIRECTORYTREE_H
#define DIRECTORYTREE_H

#include <QMouseEvent>
#include <QTreeWidget>

//
// This is just a QTreeWidget that refuses to select items with the right-mouse button
// That will help prevent the FileDialog from selecting Favorite locations when you are
// trying to get at the context menu to delete them.
//

class DirectoryTree : public QTreeWidget {
	public:
		DirectoryTree( QWidget* parent = 0 ) :
			QTreeWidget( parent ) {}
		void mousePressEvent( QMouseEvent *event ) {
			if ( event->button() == Qt::RightButton ) {
				return;
			}
			QTreeWidget::mousePressEvent( event );
		}
};

#endif  // DIRECTORYTREE_H
