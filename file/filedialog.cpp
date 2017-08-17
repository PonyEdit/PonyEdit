#include <QDebug>
#include <QDir>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

#include "file/filedialog.h"
#include "file/filelistdelegate.h"
#include "main/customtreeentry.h"
#include "main/globaldispatcher.h"
#include "main/mainwindow.h"
#include "main/tools.h"
#include "newfolderdialog.h"
#include "ssh2/serverconfigdlg.h"
#include "ssh2/sshhost.h"
#include "ssh2/sshhosttreeentry.h"
#include "syntax/syntaxdefmanager.h"
#include "ui_filedialog.h"

#define DATA_ROLE ( Qt::UserRole )
#define EXPANDED_ROLE ( Qt::UserRole + 1 )
#define TYPE_ROLE ( Qt::UserRole + 2 )
#define HOST_ROLE ( Qt::UserRole + 3 )
#define SORT_ROLE ( Qt::UserRole + 4 )

#define NODETYPE_LOCATION 1
#define NODETYPE_FAVORITE 2
#define NODETYPE_ADD_LOCATION 3
#define NODETYPE_ADD_FAVORITE 4
#define NODETYPE_LOCAL_NETWORK 5

#ifdef Q_OS_MAC
	#define UPDIR_MODIFIER Qt::ControlModifier
#else
	#define UPDIR_MODIFIER Qt::AltModifier
#endif

Location FileDialog::mLastLocation;

FileDialog::FileDialog( QWidget *parent, bool saveAs ) :
	QDialog( parent ),
	ui( new Ui::FileDialog ) {
	ui->setupUi( this );

	setWindowModality( Qt::WindowModal );

	mFileListModel = new QStandardItemModel();
	mFileListModel->setSortRole( SORT_ROLE );
	ui->fileList->setItemDelegate( new FileListDelegate( this ) );

	setAcceptDrops( true );

	mSaveAs = saveAs;
	mInEditHandler = false;
	setWindowTitle( saveAs ? tr( "Save As" ) : tr( "Open File" ) );

	ui->fileList->setModel( mFileListModel );
	ui->fileList->setShowGrid( false );
	if ( mSaveAs ) {
		ui->fileList->setSelectionMode( QAbstractItemView::SingleSelection );
	}
	ui->fileList->setWordWrap( false );
	ui->fileList->setEditTriggers( QAbstractItemView::NoEditTriggers );
	ui->fileList->setFocus();
	ui->fileList->horizontalHeader()->setHighlightSections( false );
	ui->fileList->horizontalHeader()->setSortIndicatorShown( true );

	QList< int > sizes = ui->splitter->sizes();
	sizes[0] = 1;
	sizes[1] = 300;
	ui->splitter->setSizes( sizes );

	ui->directoryTree->setContextMenuPolicy( Qt::CustomContextMenu );

	connect( ui->upLevelButton, SIGNAL( clicked() ), this, SLOT( upLevel() ) );
	connect( ui->refreshButton, SIGNAL( clicked() ), this, SLOT( refresh() ) );
	connect( ui->fileList, SIGNAL( doubleClicked( QModelIndex ) ), this, SLOT( fileDoubleClicked( QModelIndex ) ) );
	connect( gDispatcher,
	         SIGNAL( sshServersUpdated() ),
	         this,
	         SLOT( populateRemoteServers() ),
	         Qt::QueuedConnection );
	connect( ui->mainButtonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
	connect( ui->mainButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
	connect( ui->fileList->selectionModel(),
	         SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ),
	         this,
	         SLOT( fileListSelectionChanged( const QItemSelection &, const QItemSelection & ) ) );
	connect( gDispatcher,
	         SIGNAL( locationListSuccess( QList< Location >, QString ) ),
	         this,
	         SLOT( folderChildrenLoaded( QList< Location >, QString ) ),
	         Qt::QueuedConnection );
	connect( gDispatcher,
	         SIGNAL( locationListFailure( QString, QString, bool ) ),
	         this,
	         SLOT( folderChildrenFailed( QString, QString, bool ) ),
	         Qt::QueuedConnection );
	connect( this, SIGNAL( accepted() ), this, SLOT( closing() ) );
	connect( this, SIGNAL( rejected() ), this, SLOT( closing() ) );
	connect( ui->newFolderButton, SIGNAL( clicked() ), this, SLOT( createNewFolder() ) );
	connect( ui->statusWidget,
	         SIGNAL( buttonClicked( StatusWidget::Button ) ),
	         this,
	         SLOT( statusButtonClicked( StatusWidget::Button ) ) );
	connect( ui->showHidden, SIGNAL( stateChanged( int ) ), this, SLOT( refresh() ) );
	connect( ui->filterList, SIGNAL( currentIndexChanged( int ) ), this, SLOT( refresh() ) );
	connect( ui->fileName, SIGNAL( currentIndexChanged( int ) ), this, SLOT( fileNameIndexChanged() ) );
	connect( ui->fileName, SIGNAL( editTextChanged( QString ) ), this, SLOT( fileNameIndexChanged() ) );
	connect( ui->fileList->horizontalHeader(),
	         SIGNAL( sectionClicked( int ) ),
	         this,
	         SLOT( columnHeaderClicked( int ) ) );

	// Install an event filter on all children, to catch some keyboard events
	foreach ( QObject *child, children() ) {
		child->installEventFilter( this );
	}

	populateFilterList();
	populateFolderTree();
	restoreState();

	Location homeLocation( QDir::homePath() );
	Editor *editor = gMainWindow->getCurrentEditor();

	if ( NULL != editor ) {
		Location currentLoc = editor->getLocation();
		if ( currentLoc.isNull() ) {
			showLocation( mLastLocation );
		} else {
			showLocation( currentLoc.getDirectory() );
		}
	} else {
		showLocation( mLastLocation );
	}

	ui->fileName->setFocus();
}

FileDialog::~FileDialog() {
	delete mFileListModel;
	delete ui;
}

void FileDialog::restoreState() {
	QSettings settings;
	restoreGeometry( settings.value( "filedialog/geometry" ).toByteArray() );
	mSortingColumn = settings.value( "filedialog/sortColumn", QVariant( 0 ) ).toInt();
	mReverseSorting = settings.value( "filedialog/reverseSort", QVariant( false ) ).toBool();
}

void FileDialog::populateFolderTree() {
	//
	// Local computer; contains home dir and root path(s)
	//

	CustomTreeEntry *localComputer =
		new CustomTreeEntry( mIconProvider.icon( QFileIconProvider::Computer ), tr( "Local Computer" ) );
	ui->directoryTree->addTopLevelEntry( localComputer );

	Location homeLocation( QDir::homePath() );
	addLocationToTree( localComputer, homeLocation );

#ifdef Q_OS_MAC
	QDir directory( "/Volumes" );
	QStringList entries = directory.entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
	foreach ( QString entry, entries ) {
		QFileInfo driveFileInfo( "/Volumes/" + entry );
		addLocationToTree( localComputer, Location( driveFileInfo.absoluteFilePath() ) );
	}
#elif defined Q_OS_WIN
	QFileInfoList driveList = QDir::drives();
	foreach ( QFileInfo driveFileInfo, driveList ) {
		addLocationToTree( localComputer, Location( driveFileInfo.absoluteFilePath() ) );
	}
#else
	addLocationToTree( localComputer, Location( "/" ) );
#endif
	localComputer->setExpanded( true );

	//
	// Local Network (if this is a windows box)
	//

#ifdef Q_OS_WIN
	mLocalNetworkBranch = new CustomTreeEntry( mIconProvider.icon( QFileIconProvider::Network ),
	                                           tr( "Local Network" ) );
	mLocalNetworkBranch->setDelayedLoad( this, SLOT( populateWindowsShares( CustomTreeEntry * ) ) );
	ui->directoryTree->addTopLevelEntry( mLocalNetworkBranch );
#endif

	//
	// Favorite Locations; contains a list of bookmarked locations; local or otherwise
	//

	mFavoriteLocationsBranch = new CustomTreeEntry( QIcon( "icons/favorite.png" ), tr( "Favorite Locations" ) );
	ui->directoryTree->addTopLevelEntry( mFavoriteLocationsBranch );

	CustomTreeEntry *addFavorite = new CustomTreeEntry( QIcon( ":icons/add.png" ), tr( "Add Favorite..." ) );
	connect( addFavorite, SIGNAL( leftClicked( CustomTreeEntry *, QPoint ) ), this, SLOT( addToFavorites() ) );
	mFavoriteLocationsBranch->addChild( addFavorite );

	updateFavorites();
	mFavoriteLocationsBranch->setExpanded( true );

	//
	// Remote Servers; contains a list of pre-configured known servers
	//

	mRemoteServersBranch =
		new CustomTreeEntry( mIconProvider.icon( QFileIconProvider::Network ), tr( "Remote Servers" ) );
	ui->directoryTree->addTopLevelEntry( mRemoteServersBranch );

	CustomTreeEntry *addServer = new CustomTreeEntry( QIcon( ":/icons/add.png" ), tr( "Add Server..." ) );
	connect( addServer, SIGNAL( leftClicked( CustomTreeEntry *, QPoint ) ), this, SLOT( addNewServer() ) );
	mRemoteServersBranch->addChild( addServer );

	populateRemoteServers();
	mRemoteServersBranch->setExpanded( true );

	if ( mLastLocation.isNull() ) {
		mLastLocation = homeLocation;
	}
}

void FileDialog::populateRemoteServers() {
	// Take an inventory of the servers in the list now...
	QMap< SshHost *, bool > currentList;
	for ( int i = 0; i < mRemoteServersBranch->childCount(); i++ ) {
		currentList.insert( mRemoteServersBranch->child( i )->getData< SshHost * >(), false );
	}

	// Go through the list of servers that should be there. Add new entries, mark existing ones as "ok to keep"
	QList< SshHost * > knownHosts = SshHost::getKnownHosts();
	foreach ( SshHost *host, knownHosts ) {
		if ( currentList.contains( host ) ) {
			currentList.insert( host, true );
		} else {
			CustomTreeEntry *entry = new SshHostTreeEntry( host );
			mRemoteServersBranch->addChild( entry );

			connect( entry,
			         SIGNAL( leftClicked( CustomTreeEntry *, QPoint ) ),
			         this,
			         SLOT( serverClicked( CustomTreeEntry * ) ) );
		}
	}

	// Remove the list entries that have not been marked as "ok to keep"
	for ( int i = 1; i < mRemoteServersBranch->childCount(); i++ ) {
		CustomTreeEntry *child = mRemoteServersBranch->child( i );
		if ( ! currentList.value( child->getData< SshHost * >(), true ) ) {
			i--;
			delete child;
		}
	}
}

void FileDialog::serverClicked( CustomTreeEntry *entry ) {
	SshHost *host = entry->getData< SshHost * >();
	showLocation( host->getDefaultLocation() );
}

#ifdef Q_OS_WIN
void FileDialog::populateWindowsShares( CustomTreeEntry *entry ) {
	DWORD dwResult, dwResultEnum;
	HANDLE hEnum;
	DWORD cbBuffer = 16384;
	DWORD cEntries = -1;
	LPNETRESOURCE lpnrLocal;
	DWORD i;
	LPNETRESOURCE lpnr = entry->getData< LPNETRESOURCE >();

	dwResult = WNetOpenEnum( RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0, lpnr, &hEnum );
	if ( dwResult != NO_ERROR ) {
		return;
	}

	lpnrLocal = ( LPNETRESOURCE ) GlobalAlloc( GPTR, cbBuffer );
	if ( lpnrLocal == NULL ) {
		return;
	}

	do {
		ZeroMemory( lpnrLocal, cbBuffer );
		dwResultEnum = WNetEnumResource( hEnum, &cEntries, lpnrLocal, &cbBuffer );
		if ( dwResultEnum == NO_ERROR ) {
			for ( i = 0; i < cEntries; i++ ) {
				CustomTreeEntry *childEntry;
				QString name( ( QChar * ) lpnrLocal[i].lpRemoteName );
				if ( lpnrLocal[i].dwDisplayType == RESOURCEDISPLAYTYPE_SHARE ) {
					// Folder; add as a clickable location
					Location loc = Location( name );
					childEntry = addLocationToTree( entry, loc );
				} else {
					// Not a clickable location
					childEntry = new CustomTreeEntry( mIconProvider.icon(
										  QFileIconProvider::Network ),
					                                  name );
					entry->addChild( childEntry );

					if ( RESOURCEUSAGE_CONTAINER ==
					     ( lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER ) ) {
						childEntry->setData< LPNETRESOURCE >( &lpnrLocal[i] );
						populateWindowsShares( childEntry );
					}
				}
			}
		} else if ( dwResultEnum != ERROR_NO_MORE_ITEMS ) {
			break;
		}
	} while ( dwResultEnum != ERROR_NO_MORE_ITEMS );

	GlobalFree( ( HGLOBAL ) lpnrLocal );
	WNetCloseEnum( hEnum );
}

#endif

CustomTreeEntry *FileDialog::addLocationToTree( CustomTreeEntry *parent, const Location &location ) {
	CustomTreeEntry *newEntry = new CustomTreeEntry( location.getIcon(), location.getLabel() );
	newEntry->setAutoDeleteData< Location * >( new Location( location ) );
	connect( newEntry,
	         SIGNAL( leftClicked( CustomTreeEntry *, QPoint ) ),
	         this,
	         SLOT( locationClicked( CustomTreeEntry * ) ) );

	if ( location.isDirectory() ) {
		newEntry->setDelayedLoad( this, SLOT( locationExpanded( CustomTreeEntry * ) ) );
	}

	parent->addChild( newEntry );
	return newEntry;
}

void FileDialog::locationClicked( CustomTreeEntry *entry ) {
	showLocation( *entry->getData< Location * >() );
}

void FileDialog::locationExpanded( CustomTreeEntry *entry ) {
	Location *location = entry->getData< Location * >();

	mLoadingLocations.insert( location->getPath(), entry );
	location->asyncGetChildren( false );
}

void FileDialog::folderChildrenLoaded( const QList< Location > &children, const QString &locationPath ) {
	// Update the folder tree if appropriate
	CustomTreeEntry *entry = mLoadingLocations.value( locationPath, NULL );
	if ( entry ) {
		mLoadingLocations.remove( locationPath );
		entry->removeAllChildren();

		foreach ( Location childLocation, children ) {
			if ( childLocation.isDirectory() && ! childLocation.isHidden() ) {
				addLocationToTree( entry, childLocation );
			}
		}
	}

	// Update the file list if appropriate
	if ( mCurrentLocation.getPath() == locationPath ) {
		ui->fileListStack->setCurrentWidget( ui->fileListLayer );
		mFileListModel->clear();

		ui->fileName->clear();

		QStringList headerLabels;
		headerLabels.append( "Filename" );
		headerLabels.append( "Size" );
		headerLabels.append( "Last Modified" );
		mFileListModel->setHorizontalHeaderLabels( headerLabels );
		ui->fileList->setColumnWidth( 0, 250 );

		QVariant itemData = ui->filterList->itemData( ui->filterList->currentIndex() );
		QList< QVariant > filters =
			( itemData.isValid() ? ui->filterList->itemData( ui->filterList->currentIndex() ).toList() :
			  QList<
				  QVariant >() );

		foreach ( Location childLocation, children ) {
			const QString &name = childLocation.getLabel();
			if ( ! childLocation.isDirectory() && filters.length() ) {
				bool match = false;
				foreach ( QVariant filter, filters ) {
					if ( filter.toRegExp().exactMatch( name ) ) {
						match = true;
						break;
					}
				}

				if ( ! match ) {
					continue;
				}
			}

			if ( ! childLocation.isDirectory() ) {
				ui->fileName->addItem( name );
			}

			QList< QStandardItem * > row;

			QStandardItem *item = new QStandardItem();
			item->setIcon( childLocation.getIcon() );
			item->setText( name );
			item->setData( QVariant::fromValue< Location >( childLocation ), DATA_ROLE );
			item->setData( QVariant( name.toLower() ), SORT_ROLE );
			row.append( item );

			int size = childLocation.getSize();
			item = new QStandardItem();
			item->setText( childLocation.isDirectory() ? "" : Tools::humanReadableBytes( size ) );
			item->setData( QVariant( size ), SORT_ROLE );
			row.append( item );

			QDateTime lastModified = childLocation.getLastModified();
			item = new QStandardItem();
			item->setText( lastModified.toString() );
			item->setData( QVariant( lastModified ), SORT_ROLE );
			row.append( item );

			item = new QStandardItem();
			item->setData( QVariant( childLocation.isDirectory() ? 0 : 1 ), SORT_ROLE );
			row.append( item );

			mFileListModel->appendRow( row );
		}

		ui->fileName->setCurrentIndex( -1 );

		ui->fileList->resizeColumnsToContents();
		ui->fileList->resizeRowsToContents();
		ui->fileList->setColumnWidth( 0, ui->fileList->columnWidth( 0 ) + 30 );
		ui->fileList->setColumnWidth( 1, ui->fileList->columnWidth( 1 ) + 30 );
		ui->fileList->setColumnHidden( 3, true );

		applySort();

		if ( ! mSelectFile.isNull() ) {
			QList< QStandardItem * > items = mFileListModel->findItems( mSelectFile );

			QItemSelectionModel *mdl = ui->fileList->selectionModel();
			mdl->clearSelection();

			foreach ( QStandardItem *item, items ) {
				mdl->select( item->index(), QItemSelectionModel::Select );
			}

			mSelectFile = "";
		}
	}
}

void FileDialog::folderChildrenFailed( const QString &error, const QString & /*locationPath*/, bool permissionError ) {
	mFileListModel->clear();

	showStatus( QPixmap( ":/icons/error.png" ), QString( "Error: " + error ) );
	ui->statusWidget->setButtons( StatusWidget::Retry |
	                              ( mCurrentLocation.getRemoteHost() !=
	                                NULL ? StatusWidget::ShowLog : StatusWidget::None ) |
	                              ( permissionError && ! mCurrentLocation.isSudo() &&
	                                mCurrentLocation.canSudo() ? StatusWidget::SudoRetry : StatusWidget::None ) );
}

void FileDialog::columnHeaderClicked( int column ) {
	if ( mSortingColumn == column ) {
		mReverseSorting = ! mReverseSorting;
	} else {
		mSortingColumn = column;
		mReverseSorting = false;
	}
	applySort();
}

void FileDialog::applySort() {
	Qt::SortOrder order = ( mReverseSorting ? Qt::DescendingOrder : Qt::AscendingOrder );
	mFileListModel->sort( mSortingColumn, order );

	// Always sort directories separately from files.
	mFileListModel->sort( 3, order );

	ui->fileList->horizontalHeader()->setSortIndicator( mSortingColumn, order );
}

void FileDialog::statusButtonClicked( StatusWidget::Button button ) {
	switch ( button ) {
	case StatusWidget::SudoRetry:
		showLocation( mCurrentLocation.getSudoLocation() );
		break;

	case StatusWidget::Retry:
		showLocation( mCurrentLocation );
		break;

	case StatusWidget::ShowLog: {
		SshHost *host = mCurrentLocation.getRemoteHost();
		if ( host != NULL ) {
			host->showLog();
		}
		break;
	}

	default: break;
	}
}

void FileDialog::showLocation( const Location &location ) {
	if ( location.isNull() ) {
		return;
	}

	mLastLocation = location;

	ui->currentPath->setText( location.getDisplayPath() );
	mCurrentLocation = location;

	mFileListModel->clear();
	showStatus( QPixmap( ":/icons/loading.png" ), tr( "Loading ..." ) );

	mCurrentLocation.asyncGetChildren( ui->showHidden->isChecked() );

	if ( this->mSaveAs ) {
		ui->fileName->setFocus();
	} else {
		ui->fileList->setFocus();
	}
}

void FileDialog::showStatus( const QPixmap &icon, const QString &text ) {
	ui->statusWidget->setStatus( icon, text );
	ui->fileListStack->setCurrentWidget( ui->loaderLayer );
	ui->statusWidget->setButtons( StatusWidget::None );
}

void FileDialog::addNewServer() {
	SshHost::getHost( "", "" );
}

void FileDialog::keyPressEvent( QKeyEvent *event ) {
	if ( event->key() == Qt::Key_Escape ) {
		reject();
	}

	if ( focusWidget() == ui->currentPath && ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ) ) {
		if ( ui->currentPath->text() != mCurrentLocation.getDisplayPath() ) {
			Location newLocation( ui->currentPath->text() );
			showLocation( newLocation );
		}
	} else if ( ( focusWidget() == ui->fileList || focusWidget() == ui->fileName ) &&
	            ( event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return ) ) {
		QList< Location > selections = getSelectedLocations();
		if ( selections.length() > 1 || ( selections.length() > 0 && ! selections[0].isDirectory() ) ||
		     ( selections.length() == 0 && ui->fileName->currentText().length() > 0 ) ) {
			accept();
		} else if ( selections.length() > 0 && selections[0].isDirectory() ) {
			showLocation( selections[0] );
		}
	}
}

void FileDialog::upLevel() {
	Location parent = mCurrentLocation.getParent();
	if ( ! parent.isNull() ) {
		showLocation( parent );
	}
}

void FileDialog::fileDoubleClicked( QModelIndex index ) {
	QStandardItem *item = mFileListModel->itemFromIndex( index );
	int row = item->row();
	QStandardItem *primaryItem = mFileListModel->item( row, 0 );
	Location location = primaryItem->data( DATA_ROLE ).value< Location >();

	if ( location.isDirectory() ) {
		showLocation( location );
	} else {
		accept();
	}
}

void FileDialog::fileListSelectionChanged( const QItemSelection &selected, const QItemSelection & ) {
	if ( mInEditHandler ) {
		return;
	}
	mInEditHandler = true;

	QList< Location > selections = getSelectedLocations();
	QStringList selectionLabels;

	foreach ( Location l, selections ) {
		if ( ! l.isDirectory() ) {
			QString label = l.getLabel();
			label.replace( '\\', "\\\\" );
			label.replace( '"', '\"' );
			selectionLabels.append( '"' + label + '"' );
		}
	}

	ui->fileName->lineEdit()->setText( selectionLabels.join( ", " ) );

	// Ensure newly selected items are visible
	if ( selected.count() > 0 ) {
		ui->fileList->scrollTo( selected.first().topLeft() );
	}

	mInEditHandler = false;
}

QList< Location > FileDialog::getSelectedLocations() const {
	QList< Location > selections;
	QModelIndexList allSelected = ui->fileList->selectionModel()->selectedIndexes();
	foreach ( QModelIndex index, allSelected ) {
		if ( index.column() == 0 ) {
			selections.append( mFileListModel->itemFromIndex( index )->data( DATA_ROLE ).value< Location >() );
		}
	}

	return selections;
}

Location FileDialog::getNewLocation() const {
	QList< Location > selections = getSelectedLocations();
	if ( selections.length() > 0 ) {
		return selections[0];
	}

	QStringList entries = Tools::splitQuotedList( ui->fileName->currentText(), ',' );
	if ( entries.length() ) {
		return Location( mCurrentLocation.getPath() + "/" + entries[0] );
	}

	return Location();
}

void FileDialog::accept() {
	if ( mSaveAs && ui->fileName->currentText().isEmpty() ) {
		QMessageBox::information( this, "Wait a minute...", "Please enter a filename" );
		return;
	}

	if ( mSaveAs && ui->fileList->selectionModel()->selectedIndexes().length() > 0 ) {
		QMessageBox msgBox;
		msgBox.setText( tr( "This file already exists." ) );
		msgBox.setInformativeText( tr( "Are you sure you want to overwrite it?" ) );
		msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
		msgBox.setDefaultButton( QMessageBox::Yes );

		if ( msgBox.exec() == QMessageBox::No ) {
			return;
		}
	}

	QDialog::accept();
}

void FileDialog::closing() {
	// Save the geometry of this window on the way out
	QSettings settings;
	settings.setValue( "filedialog/geometry", saveGeometry() );
	settings.setValue( "filedialog/sortColumn", QVariant( mSortingColumn ) );
	settings.setValue( "filedialog/reverseSort", QVariant( mReverseSorting ) );
}

void FileDialog::addToFavorites() {
	mCurrentLocation.addToFavorites();
	updateFavorites();
}

void FileDialog::favoriteClicked( CustomTreeEntry *entry ) {
	showLocation( Location( *( entry->getData< QString * >() ) ) );
}

void FileDialog::updateFavorites() {
	// take inventory of the favorites in the list now...
	QMap< QString, bool > currentList;
	for ( int i = 0; i < mFavoriteLocationsBranch->childCount(); i++ ) {
		CustomTreeEntry *child = mFavoriteLocationsBranch->child( i );
		QString *path = child->getData< QString * >();
		if ( ! path ) {
			continue;
		}
		currentList.insert( *path, false );
	}

	// Go through the list of favorites. Add new entries, mark existing ones as keepers
	QList< Location::Favorite > favorites = Location::getFavorites();
	foreach ( const Location::Favorite &f, favorites ) {
		if ( currentList.contains( f.path ) ) {
			currentList.insert( f.path, true );
		} else {
			CustomTreeEntry *entry = new CustomTreeEntry( QIcon(), f.name );
			entry->setAutoDeleteData< QString * >( new QString( f.path ) );
			mFavoriteLocationsBranch->addChild( entry );

			connect( entry,
			         SIGNAL( leftClicked( CustomTreeEntry *, QPoint ) ),
			         this,
			         SLOT( favoriteClicked( CustomTreeEntry * ) ) );
			connect( entry,
			         SIGNAL( rightClicked( CustomTreeEntry *, QPoint ) ),
			         this,
			         SLOT( favoriteMenu( CustomTreeEntry *, QPoint ) ) );
		}
	}

	// Remove list entries that don't belong
	for ( int i = 1; i < mFavoriteLocationsBranch->childCount(); i++ ) {
		CustomTreeEntry *child = mFavoriteLocationsBranch->child( i );
		QString *path = child->getData< QString * >();
		if ( ! path ) {
			continue;
		}
		if ( ! currentList.value( *path, true ) ) {
			i--;
			delete child;
		}
	}
}

void FileDialog::favoriteMenu( CustomTreeEntry *entry, QPoint pos ) {
	pos = entry->mapToGlobal( pos );

	QString path = *( entry->getData< QString * >() );
	QMenu *contextMenu = new QMenu( this );
	QAction *deleteAction = contextMenu->addAction( tr( "Delete Favorite" ) );
	QAction *selectedAction = contextMenu->exec( pos );

	if ( selectedAction == deleteAction ) {
		Location::deleteFavorite( path );
		updateFavorites();
	}
}

void FileDialog::createNewFolder() {
	NewFolderDialog dlg( this, mCurrentLocation );
	if ( dlg.exec() ) {
		showLocation( mCurrentLocation );
	}
}

void FileDialog::dragEnterEvent( QDragEnterEvent *event ) {
	if ( event->mimeData()->hasFormat( "text/uri-list" ) ) {
		event->acceptProposedAction();
	}
}

void FileDialog::dropEvent( QDropEvent *event ) {
	QList< QUrl > urls = event->mimeData()->urls();
	if ( urls.isEmpty() ) {
		return;
	}

	QString fileName = urls.first().toLocalFile();
	if ( fileName.isEmpty() ) {
		return;
	}

	Location loc( fileName );

	if ( loc.isDirectory() ) {
		showLocation( loc );
	} else {
		showLocation( loc.getDirectory() );
		mSelectFile = loc.getLabel();
	}
}

void FileDialog::refresh() {
	showLocation( mCurrentLocation );
}

void FileDialog::populateFilterList() {
	ui->filterList->addItem( tr( "All Files" ) );

	QStringList categories = gSyntaxDefManager->getDefinitionCategories();
	foreach ( QString category, categories ) {
		QStringList filterStrings = gSyntaxDefManager->getFiltersForCategory( category );
		QList< QVariant > filterRegExps;
		foreach ( const QString &filter, filterStrings ) {
			filterRegExps.append( QVariant( QRegExp( filter, Qt::CaseInsensitive, QRegExp::Wildcard ) ) );
		}

		ui->filterList->addItem( category, filterRegExps );
	}
}

bool FileDialog::eventFilter( QObject *target, QEvent *event ) {
	if ( event->type() == QEvent::KeyPress ) {
		QKeyEvent *keyEvent = static_cast< QKeyEvent * >( event );
		if ( keyEvent->key() == Qt::Key_Up && ( keyEvent->modifiers() & UPDIR_MODIFIER ) ) {
			upLevel();
			return true;
		}
	}

	return QObject::eventFilter( target, event );
}

void FileDialog::fileNameIndexChanged() {
	if ( mInEditHandler ) {
		return;
	}
	mInEditHandler = true;

	QString text = ui->fileName->currentText();
	ui->fileList->clearSelection();

	if ( text.length() ) {
		QStringList filenames = Tools::splitQuotedList( text, ',' );

		QAbstractItemModel *model = ui->fileList->model();
		int rows = model->rowCount();
		for ( int row = 0; row < rows; row++ ) {
			if ( filenames.contains( model->data( model->index( row, 0 ) ).toString(),
			                         Qt::CaseSensitive ) ) {
				ui->fileList->selectRow( row );
			}
		}
	}

	mInEditHandler = false;
}
