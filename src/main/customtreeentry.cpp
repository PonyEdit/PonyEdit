#include <QDebug>
#include <QStylePainter>

#include "customtreeentry.h"
#include "customtreemodel.h"
#include "customtreewidget.h"

CustomTreeEntry::CustomTreeEntry( const QIcon &icon, const QString &label ) :
	mModel( NULL ),
	mParent( NULL ),
	mIndex( 0 ),
	mExpandable( false ),
	mDelayedLoad( false ),
	mStaticIcon( icon ),
	mStaticLabel( label ),
	mDataDeleteProc( NULL ),
	mData( NULL ),
	mHover( false ) {}

CustomTreeEntry::CustomTreeEntry( CustomTreeModel *model ) :
	mModel( model ),
	mParent( NULL ),
	mIndex( 0 ),
	mExpandable( false ),
	mDelayedLoad( false ),
	mStaticIcon( NULL ),
	mStaticLabel( "" ),
	mDataDeleteProc( NULL ),
	mData( NULL ),
	mHover( false ) {}

CustomTreeEntry::~CustomTreeEntry() {
	if ( mModel && mParent ) {
		mModel->beginRemoveRows( mModel->getEntryIndex( mParent ), mIndex, mIndex );
		mParent->mChildren.removeAt( mIndex );
		mParent->updateChildIndices();
		mModel->endRemoveRows();
	}

	foreach ( CustomTreeEntry *child, mChildren ) {
		child->mParent = NULL;
		child->mModel = NULL;
		delete child;
	}

	if ( mData && mDataDeleteProc ) {
		mDataDeleteProc( mData );
	}
}

void CustomTreeEntry::updateChildIndices() {
	int index = 0;
	foreach ( CustomTreeEntry *child, mChildren ) {
		child->mIndex = index++;
	}
}

void CustomTreeEntry::addChild( CustomTreeEntry *child ) {
	child->mModel = mModel;
	child->mParent = this;
	mDelayedLoad = false;

	mModel->beginInsertRows( mModel->getEntryIndex( this ), mChildren.count(), mChildren.count() );
	mChildren.append( child );
	mModel->endInsertRows();

	updateChildIndices();
}

void CustomTreeEntry::removeAllChildren() {
	if ( mChildren.length() == 0 ) {
		return;
	}

	mModel->beginRemoveRows( mModel->getEntryIndex( this ), 0, mChildren.count() - 1 );
	foreach ( CustomTreeEntry *entry, mChildren ) {
		entry->mParent = NULL;
		entry->mModel = NULL;
		delete entry;
	}
	mChildren.clear();
	mModel->endRemoveRows();
}

CustomTreeEntry *CustomTreeEntry::child( int i ) {
	handleDelayedLoad();
	return mChildren.at( i );
}

int CustomTreeEntry::childCount() {
	handleDelayedLoad();
	return mChildren.count();
}

void CustomTreeEntry::handleDelayedLoad() {
	if ( mDelayedLoad ) {
		emit expandItem( this );
		mDelayedLoad = false;
	}
}

void CustomTreeEntry::setExpanded( bool expanded ) {
	if ( expanded ) {
		mModel->mWidget->expand( mModel->getEntryIndex( this ) );
	} else {
		mModel->mWidget->collapse( mModel->getEntryIndex( this ) );
	}
}

void CustomTreeEntry::setExpandable( bool expandable ) {
	mExpandable = expandable;
}

void CustomTreeEntry::setDelayedLoad( QObject *callbackTarget, const char *loadSlot ) {
	// Disconnect any previously set delayed load actions
	disconnect( SIGNAL( expandItem( CustomTreeEntry * ) ) );

	mExpandable = mDelayedLoad = ( callbackTarget != NULL );
	if ( callbackTarget != NULL ) {
		connect( this,
		         SIGNAL( expandItem( CustomTreeEntry * ) ),
		         callbackTarget,
		         loadSlot,
		         Qt::QueuedConnection );
	}
}

void CustomTreeEntry::handleLeftClick( const QPoint &pos ) {
	emit leftClicked( this, pos );
}

void CustomTreeEntry::handleGutterClick( int gutterIconId ) {
	emit gutterIconClicked( gutterIconId );
}

void CustomTreeEntry::handleRightClick( const QPoint &pos ) {
	emit rightClicked( this, pos );
}

QPoint CustomTreeEntry::mapToGlobal( const QPoint &pos ) {
	QRect myRect = mModel->mWidget->visualRect( mModel->getEntryIndex( this ) );
	return mModel->mWidget->mapToGlobal( pos + myRect.topLeft() );
}

void CustomTreeEntry::drawIcon( QPainter *painter, QRect *area, const QIcon &icon, bool animating ) {
	QRect destination( area->left() + 2, area->top(), area->height(), area->height() );

	if ( animating ) {
		mModel->mWidget->registerAnimation( mModel->getEntryIndex( this ) );
	}

	icon.paint( painter, destination );
	area->setLeft( area->left() + area->height() + 4 );
}

void CustomTreeEntry::drawGutterText( QPainter *painter, QRect *area, const QString &text ) {
	static QTextOption to;
	to.setAlignment( Qt::AlignBottom | Qt::AlignRight );

	area->setRight( area->right() - 3 );

	QPen oldPen = painter->pen();
	painter->setPen( QColor( 128, 128, 128 ) );
	painter->drawText( *area, text, to );
	painter->setPen( oldPen );

	QFontMetrics metrics( painter->font() );
	area->setRight( area->right() - 6 - metrics.width( text ) );
}

void CustomTreeEntry::invalidate() {
	if ( mModel ) {
		mModel->invalidate( this );
	}
}

void CustomTreeEntry::addGutterIcon( int id, bool hover, const QIcon &icon, const QString &tooltip ) {
	GutterIcon gi;
	gi.id = id;
	gi.hover = hover;
	gi.icon = icon;
	gi.tooltip = tooltip;
	mGutterIcons.append( gi );
}

void CustomTreeEntry::drawGutterIcons( QPainter *painter, QRect *area ) {
	for ( QList< GutterIcon >::iterator i = mGutterIcons.begin(); i != mGutterIcons.end(); ++i ) {
		if ( isHover() || ! ( *i ).hover ) {
			area->setRight( area->right() - ( area->height() + 2 ) );
			( *i ).visibleArea.setRect( area->width() + 1, 0, area->height(), area->height() );
			( *i ).icon.paint( painter, area->right() + 1, area->top(), area->height(), area->height() );
		} else {
			( *i ).visibleArea.setRect( -1, -1, 0, 0 );
		}
	}
}

int CustomTreeEntry::gutterIconAt( const QPoint &pos ) {
	foreach ( const GutterIcon &gi, mGutterIcons ) {
		if ( gi.visibleArea.contains( pos ) ) {
			return gi.id;
		}
	}
	return -1;
}
