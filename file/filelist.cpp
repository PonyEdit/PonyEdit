#include <QDebug>
#include <QMap>
#include <QVBoxLayout>

#include "editor/editor.h"
#include "file/basefile.h"
#include "file/filelist.h"
#include "file/openfiletreeview.h"
#include "main/globaldispatcher.h"

FileList::FileList( QWidget *parent )
    : QDockWidget( parent ) {
	setWindowTitle( "Open Files" );
	QWidget *titleWidget = new QWidget( this );
	setTitleBarWidget( titleWidget );

	mTreeView = new OpenFileTreeView( this, OpenFileTreeView::CloseButtons | OpenFileTreeView::RefreshButtons );
	mTreeView->setMinimumWidth( 150 );
	setWidget( mTreeView );

	connect( gDispatcher, SIGNAL( selectFile( BaseFile * ) ), this, SLOT( selectFile( BaseFile * ) ) );
	connect( mTreeView->selectionModel(), SIGNAL( currentChanged( QModelIndex, QModelIndex ) ), this, SLOT( fileSelected() ) );
}

void FileList::selectFile( BaseFile *file ) {
	mTreeView->selectFile( file );
}

void FileList::fileSelected() {
	BaseFile *file = mTreeView->getSelectedFile();
	gDispatcher->emitSelectFile( file );
}
