#include <QDebug>
#include <QStandardItemModel>
#include <QTextBlock>

#include "file/basefile.h"
#include "file/openfilemanager.h"
#include "options/options.h"
#include "searchresultmodel.h"

SearchResultModel::SearchResultModel( QObject *parent ) :
	QAbstractItemModel( parent ),
	mRootNode( new InternalTreeNode() ),
	mCheckboxes( false ) {}

SearchResultModel::~SearchResultModel() {
	delete mRootNode;
}

void SearchResultModel::clear() {
	if ( mRootNode->children.length() == 0 ) {
		return;
	}

	beginResetModel();
	delete mRootNode;
	mRootNode = new InternalTreeNode();
	mFileNodeMap.clear();
	endResetModel();
}

SearchResultModel::InternalTreeNode *SearchResultModel::createFileNode( const Location &location ) {
	auto *newNode = new InternalTreeNode();
	newNode->parent = mRootNode;
	newNode->result.location = location;

	// Keep the locations alphabetically sorted
	int i;
	for ( i = 0; i < mRootNode->children.length(); i++ ) {
		if ( mRootNode->children[ i ]->result.location.getPath() < location.getPath() ) {
			break;
		}
	}

	beginInsertRows( QModelIndex(), i, i );
	mRootNode->children.insert( i, newNode );
	endInsertRows();

	mFileNodeMap.insert( location.getPath(), newNode );

	return newNode;
}

void SearchResultModel::addResult( const Result &result ) {
	// Find or create a filename node for the result to appear within
	InternalTreeNode *fileNode = mFileNodeMap.value( result.location.getPath() );
	if ( fileNode == nullptr ) {
		fileNode = createFileNode( result.location );
	}

	// Create a new node...
	auto *newNode = new InternalTreeNode();
	newNode->parent = fileNode;
	newNode->result = result;

	int newRow = fileNode->children.length();

	beginInsertRows( parent( createIndex( 0, 0, newNode ) ), newRow, newRow );
	fileNode->children.append( newNode );
	endInsertRows();
}

void SearchResultModel::addResults( const QList< Result > &results ) {
	foreach ( const Result &r, results ) {
		addResult( r );
	}
}

SearchResultModel::InternalTreeNode *SearchResultModel::getNodeForIndex( const QModelIndex &index ) const {
	if ( ! index.isValid() ) {
		return mRootNode;
	}

	auto *node = static_cast< InternalTreeNode * >( index.internalPointer() );
	if ( node == nullptr ) {
		return mRootNode;
	}

	return node;
}

SearchResultModel::Result *SearchResultModel::getResultForIndex( const QModelIndex &index ) {
	InternalTreeNode *node = getNodeForIndex( index );
	if ( node->result.lineNumber > -1 ) {
		return &( node->result );
	}
	return nullptr;
}

QModelIndex SearchResultModel::index( int row, int column, const QModelIndex &parent ) const {
	InternalTreeNode *parentNode = getNodeForIndex( parent );

	if ( row < 0 || row >= parentNode->children.count() ) {
		return QModelIndex();
	}

	return createIndex( row, column, parentNode->children[ row ] );
}

QModelIndex SearchResultModel::parent( const QModelIndex &child ) const {
	InternalTreeNode *thisNode = getNodeForIndex( child );
	InternalTreeNode *parentNode = thisNode->parent;
	if ( parentNode == nullptr ) {
		return QModelIndex();
	}

	InternalTreeNode *grandparentNode = parentNode->parent;

	int parentRow = 0;
	if ( grandparentNode != nullptr ) {
		parentRow = grandparentNode->children.indexOf( parentNode );
	}

	return createIndex( parentRow, 0, parentNode );
}

int SearchResultModel::rowCount( const QModelIndex &parent ) const {
	InternalTreeNode *parentNode = getNodeForIndex( parent );
	return parentNode->children.count();
}

int SearchResultModel::columnCount( const QModelIndex & /*parent*/ ) const {
	return 1;
}

QVariant SearchResultModel::data( const QModelIndex &index, int role ) const {
	if ( ! index.isValid() ) {
		return QVariant();
	}

	InternalTreeNode *node = getNodeForIndex( index );

	if ( role == Qt::DisplayRole ) {
		if ( node->parent == mRootNode ) {
			return QVariant( node->result.location.getPath() + " (" +
			                 QString::number( node->children.count() ) + ")" );
		}
	} else if ( role == Qt::FontRole ) {
		// Search results should display in the same font as the text editor
		if ( node->parent != nullptr && node->parent != mRootNode ) {
			return QVariant( *Options::EditorFont );
		}
	} else if ( role == Qt::CheckStateRole && mCheckboxes ) {
		return QVariant( node->checked );
	}

	return QVariant();
}

Qt::ItemFlags SearchResultModel::flags( const QModelIndex &index ) const {
	Qt::ItemFlags flags = QAbstractItemModel::flags( index );
	if ( mCheckboxes ) {
		flags |= Qt::ItemIsUserCheckable;
		InternalTreeNode *node = getNodeForIndex( index );
		if ( node->children.length() ) {
			flags |= Qt::ItemIsTristate;
		}
	}

	return flags;
}

void SearchResultModel::setShowCheckboxes( bool checkboxes ) {
	if ( mCheckboxes != checkboxes ) {
		beginResetModel();
		mCheckboxes = checkboxes;
		endResetModel();
	}
}

bool SearchResultModel::setData( const QModelIndex &index, const QVariant &value, int /*role*/ ) {
	Qt::CheckState checked = value.toBool() ? Qt::Checked : Qt::Unchecked;
	InternalTreeNode *node = getNodeForIndex( index );
	if ( node->parent == nullptr ) {
		return false;
	}

	if ( node->children.length() ) {
		// Apply change to self & all children
		node->checked = checked;
		foreach ( InternalTreeNode *child, node->children ) {
			child->checked = checked;
		}

		emit dataChanged( index, index );
		emit dataChanged( this->index( 0, 0, index ), this->index( 0, node->children.length() - 1, index ) );
	} else {
		// Apply change to just self and parent
		node->checked = checked;

		// Update the parent state to reflect the children.
		bool childrenUnchecked = false;
		bool childrenChecked = false;
		foreach ( InternalTreeNode *child, node->parent->children ) {
			if ( child->checked == Qt::Checked ) {
				childrenChecked = true;
			} else if ( child->checked == Qt::Unchecked ) {
				childrenUnchecked = true;
			}
			if ( childrenChecked && childrenUnchecked ) {
				break;
			}
		}
		if ( childrenChecked && childrenUnchecked ) {
			node->parent->checked = Qt::PartiallyChecked;
		} else if ( childrenChecked ) {
			node->parent->checked = Qt::Checked;
		} else {
			node->parent->checked = Qt::Unchecked;
		}

		QModelIndex parentIndex = parent( index );
		emit dataChanged( parentIndex, parentIndex );
	}

	return true;
}

void SearchResultModel::replaceSelectedResults( const QString &replacement ) {
	// go through each file
	foreach ( InternalTreeNode *locationNode, mRootNode->children ) {
		if ( locationNode->checked == Qt::Unchecked ) {
			continue;
		}

		BaseFile *file = gOpenFileManager.getFile( locationNode->result.location );
		if ( file == nullptr ) {
			continue;
		}

		QTextDocument *doc = file->getTextDocument();
		QTextCursor cursor( doc );
		cursor.beginEditBlock();

		// go through each result in each file -- backwards, to avoid changes to each entry mucking up other
		// result locations
		QList< InternalTreeNode * >::Iterator it = locationNode->children.end();
		while ( it != locationNode->children.begin() ) {
			--it;
			InternalTreeNode *resultNode = *it;
			if ( resultNode->checked == Qt::Unchecked ) {
				continue;
			}

			QTextBlock block = doc->findBlockByLineNumber( resultNode->result.lineNumber );
			if ( ! block.isValid() ) {
				continue;
			}

			cursor.setPosition( block.position() + resultNode->result.start );
			cursor.setPosition( block.position() + resultNode->result.start + resultNode->result.length,
			                    QTextCursor::KeepAnchor );

			cursor.insertText( replacement );
		}

		cursor.endEditBlock();
	}
}
