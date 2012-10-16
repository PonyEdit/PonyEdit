#include "filedlgtreemodel.h"

FileDlgTreeModel::FileDlgTreeModel(QObject *parent) :
    QStandardItemModel(parent)
{
}

virtual FileDlgTreeModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
		role = Qt::UserRole;
	else if (role == Qt::UserRole)
		role = Qt::DisplayRole;

	return QStandardItemModel::data(index, role);
}
