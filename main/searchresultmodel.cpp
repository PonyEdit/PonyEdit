#include "searchresultmodel.h"
#include <QStandardItemModel>
#include "file/basefile.h"
#include <QDebug>

SearchResultModel::SearchResultModel(QObject *parent) :
	QAbstractItemModel(parent)
{
	mRootNode = new InternalTreeNode();
}

SearchResultModel::~SearchResultModel()
{
	delete mRootNode;
}

void SearchResultModel::clear()
{
	if (mRootNode->children.length() == 0)
		return;

	beginResetModel();
	delete mRootNode;
	mRootNode = new InternalTreeNode();
	endResetModel();
}

SearchResultModel::InternalTreeNode* SearchResultModel::createFileNode(const Location& location)
{
	InternalTreeNode* newNode = new InternalTreeNode();
	newNode->parent = mRootNode;
	newNode->result.location = location;

	//	Keep the locations alphabetically sorted
	int i;
	for (i = 0; i < mRootNode->children.length(); i++)
		if (mRootNode->children[i]->result.location.getPath() < location.getPath())
			break;

	beginInsertRows(createIndex(0, 0, NULL), i, i + 1);
	mRootNode->children.insert(i, newNode);
	endInsertRows();

	mFileNodeMap.insert(location.getPath(), newNode);

	return newNode;
}

void SearchResultModel::addResult(const Result& result)
{
	//	Find or create a filename node for the result to appear within
	InternalTreeNode* fileNode = mFileNodeMap.value(result.location.getPath());
	if (fileNode == NULL)
		fileNode = createFileNode(result.location);

	//	Create a new node...
	InternalTreeNode* newNode = new InternalTreeNode();
	newNode->parent = fileNode;
	newNode->result = result;

	int newRow = fileNode->children.length();

	beginInsertRows(createIndex(0, 0, NULL), newRow, newRow + 1);
	fileNode->children.append(newNode);
	endInsertRows();
}

void SearchResultModel::addResults(const QList<Result>& results)
{
	foreach (const Result& r, results)
		addResult(r);
}

SearchResultModel::InternalTreeNode* SearchResultModel::getNodeForIndex(const QModelIndex& index) const
{
	if (!index.isValid()) return mRootNode;

	InternalTreeNode* node = static_cast<InternalTreeNode*>(index.internalPointer());
	if (node == NULL)
		return mRootNode;

	return node;
}

QModelIndex SearchResultModel::index(int row, int column, const QModelIndex& parent) const
{
	InternalTreeNode* parentNode = getNodeForIndex(parent);

	if (row < 0 || row >= parentNode->children.count())
		return QModelIndex();

	return createIndex(row, column, parentNode->children[row]);
}

QModelIndex SearchResultModel::parent(const QModelIndex& child) const
{
	InternalTreeNode* thisNode = getNodeForIndex(child);
	InternalTreeNode* parentNode = thisNode->parent;
	if (parentNode == NULL) return QModelIndex();

	InternalTreeNode* grandparentNode = parentNode->parent;

	int parentRow = 0;
	if (grandparentNode != NULL)
		parentRow = grandparentNode->children.indexOf(parentNode);

	return createIndex(parentRow, 0, parentNode);
}

int SearchResultModel::rowCount(const QModelIndex& parent) const
{
	InternalTreeNode* parentNode = getNodeForIndex(parent);
	return parentNode->children.count();
}

int SearchResultModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 1;
}

QVariant SearchResultModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	InternalTreeNode* node = getNodeForIndex(index);

	if (role == Qt::DisplayRole)
	{
		if (node->parent == mRootNode)
			return QVariant(node->result.location.getPath() + " (" + QString::number(node->children.count()) + ")");
		else if (node->parent != NULL)
			return QVariant(QString::number(node->result.lineNumber) + ": " + node->result.matchedLine);
	}

	return QVariant();
}





