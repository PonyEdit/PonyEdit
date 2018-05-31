#include <QDebug>
#include <QMessageBox>

#include "basefile.h"
#include "openfilemanager.h"
#include "openfiletreemodel.h"
#include "openfiletreeview.h"
#include "unsavedchangesdialog.h"

OpenFileTreeModel::OpenFileTreeModel( QObject *parent,
                                      int flags,
                                      const QList< BaseFile * > *files ) :
	QAbstractItemModel( parent ) {
	mOptionFlags = flags;
	mTopLevelNode = new Node( Root );
	mParent = reinterpret_cast< OpenFileTreeView * >( parent );

	mExplicitFiles = ( files != nullptr );
	if ( mExplicitFiles ) {
		mFiles = *files;

		foreach ( BaseFile *file, mFiles ) {
			fileOpened( file );
		}
	} else {
		connect( &gOpenFileManager, SIGNAL( fileOpened( BaseFile * ) ), this, SLOT( fileOpened( BaseFile * ) ) );
		connect( &gOpenFileManager,
		         SIGNAL( fileClosed( BaseFile * ) ),
		         this,
		         SLOT( fileClosed( BaseFile * ) ),
		         Qt::DirectConnection );

		// Add any already-open files
		foreach ( BaseFile *file, gOpenFileManager.getOpenFiles() ) {
			fileOpened( file );
		}
	}
}

OpenFileTreeModel::~OpenFileTreeModel() {
	delete mTopLevelNode;
}

QString OpenFileTreeModel::Node::getLabel() {
	switch ( level ) {
		case Host:
			return location.getHostName();

		case Directory:
			return location.getHostlessPath();

		case File:
			return location.getLabel();

		case Root:
			return "";
	}
}

OpenFileTreeModel::Node *OpenFileTreeModel::Node::findChildNode( const QString &label ) {
	foreach ( Node *node, children ) {
		if ( node->getLabel() == label ) {
			return node;
		}
	}
	return nullptr;
}

OpenFileTreeModel::Node *OpenFileTreeModel::Node::findChildNode( const Location &loc ) {
	foreach ( Node *node, children ) {
		if ( node->location == loc ) {
			return node;
		}
	}
	return nullptr;
}

OpenFileTreeModel::Node *OpenFileTreeModel::Node::findChildNode( BaseFile *f ) {
	foreach ( Node *node, children ) {
		if ( node->file == f ) {
			return node;
		}
	}
	return nullptr;
}

QModelIndex OpenFileTreeModel::getNodeIndex( Node *node ) const {
	Node *parentNode = node->parent;
	if ( ! parentNode ) {
		return QModelIndex();
	}
	return createIndex( parentNode->children.indexOf( node ), 0, static_cast< void * >( node ) );
}

void OpenFileTreeModel::addNodeToTree( Node *parentNode, Node *node ) {
	QModelIndex parentIndex = getNodeIndex( parentNode );

	// Work out where within the parent to insert this new item... (Sorting alphabetically)
	int insertIndex;
	for ( insertIndex = 0; insertIndex < parentNode->children.length(); insertIndex++ ) {
		if ( parentNode->children[ insertIndex ]->location.getPath() > node->location.getPath() ) {
			break;
		}
	}

	// Lock the model, insert the row, unlock the model.
	beginInsertRows( parentIndex, insertIndex, insertIndex );
	parentNode->children.insert( insertIndex, node );
	node->parent = parentNode;
	endInsertRows();
}

OpenFileTreeModel::Node *OpenFileTreeModel::getHostNode( const Location &location ) {
	// See if there is already an Node for this location.
	Node *existingNode = mTopLevelNode->findChildNode( location.getHostName() );
	if ( existingNode ) {
		return existingNode;
	}

	Node *newNode = new Node( Host );
	newNode->location = location;
	addNodeToTree( mTopLevelNode, newNode );
	return newNode;
}

OpenFileTreeModel::Node *OpenFileTreeModel::getDirectoryNode( const Location &location ) {
	// New files don't show directory subtrees.
	if ( location.getPath().isEmpty() ) {
		return getHostNode( location );
	}

	// Find / Create a node for the host this directory is on
	Node *hostNode = getHostNode( location );

	// See if there is already a Node for this directory
	Node *existingNode = hostNode->findChildNode( location );
	if ( existingNode ) {
		return existingNode;
	}

	Node *newNode = new Node( Directory );
	newNode->location = location;
	addNodeToTree( hostNode, newNode );
	return newNode;
}

void OpenFileTreeModel::fileOpened( BaseFile *file ) {
	// Find / Create a node for the directory this file is in
	Node *directoryNode = getDirectoryNode( file->getLocation().getDirectory() );

	// Create a Node for the new file
	Node *newNode = new Node( File );
	newNode->file = file;
	newNode->location = file->getLocation();
	addNodeToTree( directoryNode, newNode );
	mFileLookup.insert( file, newNode );

	mParent->expandAll();

	connect( file, SIGNAL( openStatusChanged( int ) ), this, SLOT( fileChanged() ) );
	connect( file, SIGNAL( fileProgress( int ) ), this, SLOT( fileChanged() ) );
	connect( file, SIGNAL( unsavedStatusChanged() ), this, SLOT( fileChanged() ) );
}

void OpenFileTreeModel::fileClosed( BaseFile *file ) {
	removeFile( file );
}

void OpenFileTreeModel::fileChanged() {
	BaseFile *file = reinterpret_cast< BaseFile * >( QObject::sender() );
	Node *fileNode = mFileLookup.value( file );
	if ( fileNode ) {
		QModelIndex index = getNodeIndex( fileNode );
		if ( index.isValid() ) {
			emit dataChanged( index, index.sibling( index.row(), 1 ) );
		}
	}
}

QModelIndex OpenFileTreeModel::index( int row, int column, const QModelIndex &parent ) const {
	Node *parentNode = ( parent.isValid() ? static_cast< Node * >( parent.internalPointer() ) : mTopLevelNode );
	if ( parentNode->children.length() <= row || 0 > row ) {
		return QModelIndex();
	}
	Node *Node = parentNode->children[ row ];

	return createIndex( row, column, Node );
}

QModelIndex OpenFileTreeModel::parent( const QModelIndex &index ) const {
	Node *node = static_cast< Node * >( index.internalPointer() );
	if ( ! node || ! node->parent || node->parent == mTopLevelNode ) {
		return QModelIndex();
	}

	Node *parentNode = node->parent;
	return getNodeIndex( parentNode );
}

int OpenFileTreeModel::rowCount( const QModelIndex &parent ) const {
	Node *parentNode = ( parent.isValid() ? static_cast< Node * >( parent.internalPointer() ) : mTopLevelNode );
	return parentNode->children.length();
}

int OpenFileTreeModel::columnCount( const QModelIndex & /*parent*/ ) const {
	int cols = 1;

	if ( mOptionFlags & OpenFileTreeView::CloseButtons ) {
		cols++;
	}
	if ( mOptionFlags & OpenFileTreeView::RefreshButtons ) {
		cols++;
	}

	return cols;
}

QVariant OpenFileTreeModel::data( const QModelIndex &index, int role ) const {
	if ( ! index.isValid() ) {
		return QVariant();
	}

	Node *node = static_cast< Node * >( index.internalPointer() );
	if ( role == Qt::ToolTipRole ) {
		QString tooltip = node->location.getPath();
		if ( node->file ) {
			if ( node->file->hasUnsavedChanges() ) {
				tooltip += "\nUnsaved Changes";
			}
			tooltip += QString( "\n" ) + node->file->sStatusLabels[ node->file->getOpenStatus() ];
		}
		return QVariant( tooltip );
	} else if ( role < Qt::UserRole ) {
		return QVariant();
	}

	switch ( role ) {
		case LocationRole:
			return QVariant::fromValue< Location >( node->location );

		case FileRole:
			return QVariant::fromValue< void * >( node->file );

		case TypeRole:
			return QVariant( static_cast< int >( node->level ) );

		case LabelRole:
			return node->getLabel();
	}

	return QVariant();
}

Qt::ItemFlags OpenFileTreeModel::flags( const QModelIndex &index ) const {
	Node *node = static_cast< Node * >( index.internalPointer() );
	if ( ! node ) {
		return Qt::NoItemFlags;
	}
	return ( node->file ? Qt::ItemIsSelectable |
	         ( index.column() == 0 ? Qt::ItemIsEnabled : Qt::NoItemFlags ) : Qt::NoItemFlags );
}

QModelIndex OpenFileTreeModel::findFile( BaseFile *file ) const {
	Node *fileNode = mFileLookup.value( file );
	if ( fileNode && fileNode->parent ) {
		int row = fileNode->parent->children.indexOf( fileNode );
		if ( row >= 0 ) {
			return createIndex( row, 0, fileNode );
		}
	}

	return QModelIndex();
}

void OpenFileTreeModel::removeNode( const QModelIndex &index ) {
	if ( ! index.isValid() ) {
		return;
	}

	Node *node = reinterpret_cast< Node * >( index.internalPointer() );
	if ( ! node ) {
		return;
	}

	Node *parentNode = node->parent;
	if ( ! parentNode ) {
		return;
	}

	int row = parentNode->children.indexOf( node );
	QModelIndex parentIndex = parent( index );

	beginRemoveRows( parentIndex, row, row );
	parentNode->children.removeAt( row );
	endRemoveRows();

	mFileLookup.remove( node->file );
	delete node;

	// If the parent is not the top level, remove if empty
	if ( parentNode->level != Root && parentNode->children.length() == 0 ) {
		removeNode( parentIndex );
	}
}

BaseFile *OpenFileTreeModel::getFileAtIndex( const QModelIndex &index ) {
	if ( index.isValid() ) {
		Node *node = static_cast< Node * >( index.internalPointer() );
		if ( node ) {
			return node->file;
		}
	}

	return nullptr;
}

QList< BaseFile * > OpenFileTreeModel::getIndexAndChildFiles( Node *node ) {
	QList< BaseFile * > files;
	if ( ! node ) {
		return files;
	}

	if ( node->file ) {
		files.append( node->file );
	}

	foreach ( Node *childNode, node->children ) {
		files.append( getIndexAndChildFiles( childNode ) );
	}

	return files;
}

QList< BaseFile * > OpenFileTreeModel::getIndexAndChildFiles( const QModelIndex &index ) {
	if ( ! index.isValid() ) {
		return QList< BaseFile * >();
	}
	return getIndexAndChildFiles( static_cast< Node * >( index.internalPointer() ) );
}

void OpenFileTreeModel::removeFile( BaseFile *file ) {
	QModelIndex index = findFile( file );
	if ( index.isValid() ) {
		// We need to remove a node from the tree.
		removeNode( index );
	}

	mFiles.removeAll( file );
}
