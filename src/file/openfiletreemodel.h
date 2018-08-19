#ifndef OPENFILEMODEL_H
#define OPENFILEMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include "location.h"
#include "openfiletreeview.h"

class BaseFile;

class OpenFileTreeModel : public QAbstractItemModel {
	Q_OBJECT

	public:
		enum Roles { LocationRole = Qt::UserRole, FileRole = Qt::UserRole + 1, TypeRole = Qt::UserRole + 2,
			     LabelRole = Qt::UserRole + 3 };
		enum Level { Root, Host, Directory, File };

		// Displays explicitFiles if specified. If left NULL, gets a list of all currently open files.
		OpenFileTreeModel( QObject *parent, int flags, const QList< BaseFile * > *explicitFiles = nullptr );
		~OpenFileTreeModel();

		QModelIndex index( int row, int column, const QModelIndex &parent ) const;
		QModelIndex parent( const QModelIndex &index ) const;
		int rowCount( const QModelIndex &parent = QModelIndex() ) const;
		int columnCount( const QModelIndex &parent = QModelIndex() ) const;
		QVariant data( const QModelIndex &index, int role ) const;
		Qt::ItemFlags flags( const QModelIndex &index ) const;

		QModelIndex findFile( BaseFile *file ) const;
		BaseFile *getFileAtIndex( const QModelIndex &index );
		QList< BaseFile * > getIndexAndChildFiles( const QModelIndex &index );

		// Only useful when explicitly specified files.
		void removeFile( BaseFile *file );

	private slots:
		void fileOpened( BaseFile *file );
		void fileClosed( BaseFile *file );
		void fileChanged();

	private:
		class Node {
			public:
				Node( Level l ) :
					level( l ),
					parent( nullptr ),
					file( nullptr ) {}
				Node *findChildNode( const QString &label );
				Node *findChildNode( const Location &loc );
				Node *findChildNode( BaseFile *f );
				QString getLabel();

				Level level;
				Node *parent;
				BaseFile *file;
				Location location;
				QList< Node * > children;
		};

		QModelIndex getNodeIndex( Node *node ) const;
		void addNodeToTree( Node *parentNode, Node *node );
		Node *getHostNode( const Location &location );
		Node *getDirectoryNode( const Location &location );
		void removeNode( const QModelIndex &index );
		QList< BaseFile * > getIndexAndChildFiles( Node *node );

		QList< BaseFile * > mFiles;      // Used if a list of files explicitly supplied
		Node *mTopLevelNode;
		QMap< BaseFile *, Node * > mFileLookup;
		void dumpNodes( Node *node );
		OpenFileTreeView *mParent;

		int mOptionFlags;
		bool mExplicitFiles;
};

#endif  // OPENFILEMODEL_H
