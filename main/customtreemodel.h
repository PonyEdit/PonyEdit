#ifndef CUSTOMTREEMODEL_H
#define CUSTOMTREEMODEL_H

#include <QAbstractItemModel>

class CustomTreeEntry;
class CustomTreeModel;
class CustomTreeWidget;
class CustomTreeModel : public QAbstractItemModel {
	friend class CustomTreeEntry;
	Q_OBJECT

public:
	explicit CustomTreeModel( CustomTreeWidget *widget );
	~CustomTreeModel();

	inline CustomTreeWidget *getWidget() const {
		return mWidget;
	}

	QModelIndex index( int row, int column, const QModelIndex &parent ) const;
	QModelIndex parent( const QModelIndex &index ) const;
	int         rowCount( const QModelIndex &index ) const;
	int         columnCount( const QModelIndex &index ) const;
	QVariant    data( const QModelIndex &index, int role ) const;
	bool        hasChildren( const QModelIndex &parent ) const;

	CustomTreeEntry *getEntry( const QModelIndex &index ) const;
	void             addTopLevelEntry( CustomTreeEntry *entry );
	QModelIndex      getEntryIndex( CustomTreeEntry *entry ) const;

	void invalidate( CustomTreeEntry *entry );

private:
	CustomTreeEntry * mRootNode;
	CustomTreeWidget *mWidget;
};

#endif // CUSTOMTREEMODEL_H
