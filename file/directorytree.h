#ifndef DIRECTORYTREE_H
#define DIRECTORYTREE_H

#include <QTreeWidget>
#include <QMouseEvent>

class DirectoryTree : public QTreeWidget
{
public:
	DirectoryTree(QWidget* parent = 0) : QTreeWidget(parent) {}
	void mousePressEvent(QMouseEvent *event)
	{
		if (event->button() == Qt::RightButton)
			return;
		QTreeWidget::mousePressEvent(event);
	}
};

#endif // DIRECTORYTREE_H
