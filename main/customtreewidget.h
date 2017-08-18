#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

#include <QTreeView>

class CustomTreeEntry;
class CustomTreeModel;
class CustomTreeDelegate;
class CustomTreeWidget : public QTreeView {
	Q_OBJECT

	public:
		explicit CustomTreeWidget( QWidget *parent = 0 );
		~CustomTreeWidget();

		void addTopLevelEntry( CustomTreeEntry *entry );
		void mousePressEvent( QMouseEvent *event );

		virtual void paintEvent( QPaintEvent *event );
		virtual void timerEvent( QTimerEvent *event );
		void registerAnimation( const QModelIndex &area );

	private slots:
		void entryClicked( QModelIndex index );

	private:
		CustomTreeModel *mModel;
		CustomTreeDelegate *mDelegate;
		QPoint mLastClickedPoint;

		int mAnimationTimerId;
		QList< QModelIndex > mAnimatingIndices;
};

#endif  // CUSTOMTREEWIDGET_H
