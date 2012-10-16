#include "customtreemodel.h"
#include "customtreeentry.h"
#include "customtreewidget.h"

CustomTreeModel::CustomTreeModel(CustomTreeWidget* widget) : QAbstractItemModel()
{
	mWidget = widget;
	mRootNode = new CustomTreeEntry(this);
}

CustomTreeModel::~CustomTreeModel()
{
	delete mRootNode;
}

QModelIndex CustomTreeModel::index(int row, int column, const QModelIndex& parent) const
{
	CustomTreeEntry* parentEntry = getEntry(parent);

	if (parentEntry->childCount() <= row)
		return QModelIndex();

	CustomTreeEntry* child = parentEntry->child(row);
	return createIndex(row, column, static_cast<void*>(child));
}

QModelIndex CustomTreeModel::getEntryIndex(CustomTreeEntry *entry) const
{
	CustomTreeEntry* parent = entry->getParent();

	if (parent == NULL)
		return createIndex(0, 0, static_cast<void*>(entry));
	else
		return createIndex(entry->getIndexWithinParent(), 0, static_cast<void*>(entry));
}

QModelIndex CustomTreeModel::parent(const QModelIndex& index) const
{
	CustomTreeEntry* entry = getEntry(index);
	CustomTreeEntry* parent = entry->getParent();

	if (parent == NULL)
		return QModelIndex();

	return getEntryIndex(parent);
}

bool CustomTreeModel::hasChildren(const QModelIndex &parent) const
{
	return getEntry(parent)->isExpandable();
}

int CustomTreeModel::rowCount(const QModelIndex& index) const
{
	return getEntry(index)->childCount();
}

int CustomTreeModel::columnCount(const QModelIndex& /*index*/) const
{
	return 1;
}

QVariant CustomTreeModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::DecorationRole || role == Qt::DisplayRole)
	{
		CustomTreeEntry* entry = getEntry(index);
		if (!entry->isCustomDrawn())
		{
			if (role == Qt::DecorationRole)
				return entry->getIcon();
			else
				return entry->getLabel();
		}
	}

	return QVariant();
}

CustomTreeEntry* CustomTreeModel::getEntry(const QModelIndex& index) const
{
	CustomTreeEntry* entry = static_cast<CustomTreeEntry*>(index.internalPointer());
	return entry ? entry : mRootNode;
}

void CustomTreeModel::addTopLevelEntry(CustomTreeEntry *entry)
{
	mRootNode->addChild(entry);
}

void CustomTreeModel::invalidate(CustomTreeEntry* entry)
{
	QModelIndex index = getEntryIndex(entry);
	emit dataChanged(index, index);
}
