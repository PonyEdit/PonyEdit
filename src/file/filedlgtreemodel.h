#ifndef FILEDLGTREEMODEL_H
#define FILEDLGTREEMODEL_H

#include <QStandardItemModel>

class FileDlgTreeModel : public QStandardItemModel {
	Q_OBJECT

	public:
		explicit FileDlgTreeModel( QObject *parent = 0 );

		virtual QVariant data( const QModelIndex &index, int role ) const;
};

#endif  // FILEDLGTREEMODEL_H
