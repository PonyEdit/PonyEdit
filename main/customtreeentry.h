#ifndef CUSTOMTREEENTRY_H
#define CUSTOMTREEENTRY_H

#include <QIcon>
#include <QObject>
#include <QPoint>
#include <QStyleOptionViewItem>

class CustomTreeModel;
class QStylePainter;
class CustomTreeEntry : public QObject
{
Q_OBJECT

public:
explicit CustomTreeEntry( const QIcon& icon, const QString& label );
explicit CustomTreeEntry( CustomTreeModel* model );
~CustomTreeEntry();

// Child management
inline CustomTreeEntry* getParent() const {
	return mParent;
}

void addChild( CustomTreeEntry* child );
CustomTreeEntry* child( int i );
int childCount();
void removeAllChildren();
inline int getIndexWithinParent() const {
	return mIndex;
}

// Clicks and pixel position mapping
void handleLeftClick( const QPoint& pos );
void handleRightClick( const QPoint& pos );
void handleGutterClick( int gutterIconId );
QPoint mapToGlobal( const QPoint& pos );
inline void setHover( bool hover ) {
	mHover = hover;
}

inline bool isHover() {
	return mHover;
}

int gutterIconAt( const QPoint& pos );

// Data handling methods. Use setAutoDeleteData if you want the data to be deleted with the CustomTreeEntry.
template < class T > void setAutoDeleteData( T data ) {
	mDataDeleteProc = ( deleteData< T >); mData = static_cast< void* >( data );
}

template < class T > void setData( T data ) {
	mDataDeleteProc = NULL; mData = static_cast< void* >( data );
}

template < class T > T getData() const {
	return static_cast< T >( mData );
}

// Expansion / collapsing stuff
void setExpanded( bool expanded );
void setExpandable( bool expandable );
inline bool isExpandable() const {
	return mExpandable || mChildren.length();
}

void setDelayedLoad( QObject* callbackTarget, const char* loadSlot );

// What I look like
inline const QIcon& getIcon() const {
	return mStaticIcon;
}

inline const QString& getLabel() const {
	return mStaticLabel;
}

virtual bool isCustomDrawn() const {
	return false;
}

virtual void customDraw( QPainter* /*painter*/, const QStyleOptionViewItem& /*option*/ ) {}
void drawIcon( QPainter* painter, QRect* area, const QIcon& icon, bool animating = false );
void drawGutterText( QPainter* painter, QRect* area, const QString& text );

void addGutterIcon( int id, bool hover, const QIcon& icon, const QString& tooltip );
void drawGutterIcons( QPainter* painter, QRect* area );

signals:
void leftClicked( CustomTreeEntry* entry, QPoint pos );
void rightClicked( CustomTreeEntry* entry, QPoint pos );
void expandItem( CustomTreeEntry* entry );
void gutterIconClicked( int iconId );

public slots:
void invalidate();

private:
struct GutterIcon { int id; bool hover; QIcon icon; QString tooltip; QRect visibleArea; };

void init();
template < class T > static void deleteData( void* data ) {
	delete static_cast< T >( data );
}

void handleDelayedLoad();
void updateChildIndices();

CustomTreeModel* mModel;
CustomTreeEntry* mParent;
int mIndex;		// Index within parent; for faster preparation of QModelIndices.
QList< CustomTreeEntry* > mChildren;
bool mExpandable;
bool mDelayedLoad;

QIcon mStaticIcon;
QString mStaticLabel;
QList< GutterIcon > mGutterIcons;

typedef void (*DeleteProc)( void* );
DeleteProc mDataDeleteProc;
void* mData;

bool mHover;
};

#endif	// CUSTOMTREEENTRY_H
