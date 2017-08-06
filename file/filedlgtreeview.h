#ifndef FILEDLGTREEVIEW_H
#define FILEDLGTREEVIEW_H

#include <QTreeView>

class FileDlgTreeView : public QTreeView {
	Q_OBJECT
public:
	explicit FileDlgTreeView( QObject *parent = 0 );

	void mousePressEvent( QMouseEvent *event );
};

#endif // FILEDLGTREEVIEW_H
