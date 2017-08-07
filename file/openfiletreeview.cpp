#include <QDebug>
#include <QHeaderView>

#include "openfileitemdelegate.h"
#include "openfilemanager.h"
#include "openfiletreemodel.h"
#include "openfiletreeview.h"

OpenFileTreeView::OpenFileTreeView( QWidget *parent, int optionFlags, const QList< BaseFile* >* files ) :
	QTreeView( parent ) {
	mExtraColumns = 0;
	mRefreshColumn = 0;
	mCloseColumn = 0;

	// Create & attach the model; the model supplies the data in tree layout to display
	mModel = new OpenFileTreeModel( this, optionFlags, files );
	setModel( mModel );

	// Create & attach the item delegate; it handles drawing each item
	mDelegate = new OpenFileItemDelegate( this );
	setItemDelegate( mDelegate );

	// Configure look & feel details
	header()->hide();
	setAttribute( Qt::WA_MacShowFocusRect, false );
	viewport()->setAttribute( Qt::WA_Hover );
	header()->setSectionResizeMode( 0, QHeaderView::Stretch );
	setSelectionMode(
		optionFlags & MultiSelect ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection );
	header()->setStretchLastSection( false );

	if ( optionFlags & RefreshButtons ) {
		mRefreshColumn = ++mExtraColumns;

		header()->setSectionResizeMode( mRefreshColumn, QHeaderView::Fixed );
		header()->resizeSection( mRefreshColumn, 16 );
	}

	if ( optionFlags & CloseButtons ) {
		mCloseColumn = ++mExtraColumns;

		header()->setSectionResizeMode( mCloseColumn, QHeaderView::Fixed );
		header()->resizeSection( mCloseColumn, 16 );
	}

	if ( mExtraColumns ) {
		connect( this, SIGNAL( clicked( QModelIndex ) ), this, SLOT( itemClicked( QModelIndex ) ) );
	} else {
		header()->setStretchLastSection( true );
	}
}

void OpenFileTreeView::dataChanged( const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/,
                                    const QVector< int >& /*roles*/ ) {
	this->expandAll();
}

BaseFile* OpenFileTreeView::getSelectedFile() const {
	QModelIndex i = this->selectionModel()->currentIndex();
	return mModel->getFileAtIndex( i );
}

void OpenFileTreeView::selectFile( BaseFile* file ) {
	if ( ! file ) {
		return;
	}
	if ( getSelectedFile() != file ) {
		QModelIndex index = mModel->findFile( file );
		setCurrentIndex( index );
	}
}

void OpenFileTreeView::itemClicked( QModelIndex index ) {
	if ( ! index.isValid() ) {
		return;
	}

	if ( mRefreshColumn && index.column() == mRefreshColumn ) {
		// Refresh button clicked.
		QList< BaseFile* > refreshingFiles = mModel->getIndexAndChildFiles( index );
		gOpenFileManager.refreshFiles( refreshingFiles );
	}

	if ( mCloseColumn && index.column() == mCloseColumn ) {
		// Close button clicked.
		QList< BaseFile* > closingFiles = mModel->getIndexAndChildFiles( index );
		gOpenFileManager.closeFiles( closingFiles );
	}
}

QList< BaseFile* > OpenFileTreeView::getSelectedFiles() const {
	QModelIndexList selectedIndices = selectionModel()->selectedIndexes();
	QList< BaseFile* > selectedFiles;
	foreach( QModelIndex index, selectedIndices ) {
		BaseFile* file = mModel->getFileAtIndex( index );
		if ( file != NULL ) {
			selectedFiles.append( file );
		}
	}

	return selectedFiles;
}

void OpenFileTreeView::removeFile( BaseFile* file ) {
	mModel->removeFile( file );
}
