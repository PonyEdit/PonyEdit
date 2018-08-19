#include <QDebug>
#include <QMouseEvent>

#include "customtreedelegate.h"
#include "customtreeentry.h"
#include "customtreemodel.h"
#include "customtreewidget.h"

CustomTreeWidget::CustomTreeWidget( QWidget *parent ) :
	QTreeView( parent ) {
	mAnimationTimerId = 0;

	mModel = new CustomTreeModel( this );
	setModel( mModel );

	mDelegate = new CustomTreeDelegate( mModel );
	setItemDelegate( mDelegate );

	setAttribute( Qt::WA_MacShowFocusRect, false );
	viewport()->setAttribute( Qt::WA_Hover );

	connect( this, SIGNAL(clicked(QModelIndex)), this, SLOT(entryClicked(QModelIndex)) );
}

CustomTreeWidget::~CustomTreeWidget() {
	delete mModel;
	delete mDelegate;
}

void CustomTreeWidget::addTopLevelEntry( CustomTreeEntry *entry ) {
	mModel->addTopLevelEntry( entry );
}

void CustomTreeWidget::mousePressEvent( QMouseEvent *event ) {
	// Work out what was clicked.
	QModelIndex index      = indexAt( event->pos() );
	CustomTreeEntry *entry = mModel->getEntry( index );
	mLastClickedPoint = event->pos() - visualRect( index ).topLeft();

	// Pass right clicks though to entry, stop there.
	if ( event->button() == Qt::RightButton ) {
		if ( entry ) {
			entry->handleRightClick( mLastClickedPoint );
		}
		return;
	}

	// Handle clicks on gutter icons without allowing the item to be selected
	if ( entry ) {
		int gutterIcon = entry->gutterIconAt( mLastClickedPoint );
		if ( gutterIcon > -1 ) {
			entry->handleGutterClick( gutterIcon );
			return;
		}
	}

	// Ok, handle as normal.
	QTreeView::mousePressEvent( event );
}

void CustomTreeWidget::entryClicked( QModelIndex index ) {
	CustomTreeEntry *entry = mModel->getEntry( index );
	if ( entry ) {
		entry->handleLeftClick( mLastClickedPoint );
	}
}

void CustomTreeWidget::paintEvent( QPaintEvent *event ) {
	mAnimatingIndices.clear();
	QTreeView::paintEvent( event );

	bool animating = mAnimatingIndices.count() > 0;
	if ( animating && ! mAnimationTimerId ) {
		mAnimationTimerId = startTimer( 100 );
	} else if ( ! animating && mAnimationTimerId ) {
		killTimer( mAnimationTimerId );
		mAnimationTimerId = 0;
	}
}

void CustomTreeWidget::registerAnimation( const QModelIndex &index ) {
	mAnimatingIndices.append( index );
}

void CustomTreeWidget::timerEvent( QTimerEvent *event ) {
	if ( event->timerId() == mAnimationTimerId ) {
		foreach ( const QModelIndex &index, mAnimatingIndices ) {
			update( index );
		}
	} else {
		QTreeView::timerEvent( event );
	}
}
