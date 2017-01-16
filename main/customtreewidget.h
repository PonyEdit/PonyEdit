#ifndef CUSTOMTREEWIDGET_H
#define CUSTOMTREEWIDGET_H

HIDE_COMPILE_WARNINGS

#include <QTreeView>

UNHIDE_COMPILE_WARNINGS


class CustomTreeEntry;
class CustomTreeModel;
class CustomTreeDelegate;
class CustomTreeWidget : public QTreeView
{
    Q_OBJECT
public:
	explicit CustomTreeWidget(QWidget *parent = 0);
	~CustomTreeWidget();

	CustomTreeWidget(CustomTreeWidget const&) = delete;
	CustomTreeWidget& operator=(CustomTreeWidget const&) = delete;
	
	void addTopLevelEntry(CustomTreeEntry* entry);
	void mousePressEvent(QMouseEvent *event);

	virtual void paintEvent(QPaintEvent* event);
	virtual void timerEvent(QTimerEvent* event);
	void registerAnimation(const QModelIndex& area);

private slots:
	void entryClicked(QModelIndex index);

private:
	CustomTreeModel* mModel;
	CustomTreeDelegate* mDelegate;
	QPoint mLastClickedPoint;

	int mAnimationTimerId;
	QList<QModelIndex> mAnimatingIndices;
};

#endif // CUSTOMTREEWIDGET_H
