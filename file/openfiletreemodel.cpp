#include "openfiletreeview.h"
#include "unsavedchangesdialog.h"
#include "openfiletreemodel.h"
#include "openfilemanager.h"
#include "basefile.h"
#include <QDebug>
#include <QMessageBox>

OpenFileTreeModel::OpenFileTreeModel(QObject* parent, int flags, const QList<BaseFile*>* files) : QAbstractItemModel(parent)
{
	mOptionFlags = flags;
	mTopLevelEntry = new Entry();

	mExplicitFiles = (files != NULL);
	if (mExplicitFiles)
	{
		mFiles = *files;

		foreach (BaseFile* file, mFiles)
			fileOpened(file);
	}
	else
	{
		connect(&gOpenFileManager, SIGNAL(fileOpened(BaseFile*)), this, SLOT(fileOpened(BaseFile*)));
		connect(&gOpenFileManager, SIGNAL(fileClosed(BaseFile*)), this, SLOT(fileClosed(BaseFile*)), Qt::DirectConnection);

		//	Add any already-open files
		foreach (BaseFile* file, gOpenFileManager.getOpenFiles())
			fileOpened(file);
	}
}

OpenFileTreeModel::~OpenFileTreeModel()
{
	delete mTopLevelEntry;
}

void OpenFileTreeModel::fileOpened(BaseFile* file)
{
	//	Create an Entry for the new file
	Entry* newEntry = new Entry();
	newEntry->file = file;
	newEntry->location = file->getLocation();
	mFileLookup.insert(file, newEntry);

	//	Work out where this belongs in the tree... (create a directory branch if necessary)
	QModelIndex directoryIndex = registerDirectory(file->getDirectory());

	//	Insert it.
	addToTree(directoryIndex, newEntry);

	connect(file, SIGNAL(openStatusChanged(int)), this, SLOT(fileChanged()));
	connect(file, SIGNAL(fileOpenProgress(int)), this, SLOT(fileChanged()));
	connect(file, SIGNAL(unsavedStatusChanged()), this, SLOT(fileChanged()));
}

void OpenFileTreeModel::fileClosed(BaseFile* file)
{
	removeFile(file);
}

void OpenFileTreeModel::fileChanged()
{
	BaseFile* file = (BaseFile*)QObject::sender();
	Entry* fileEntry = mFileLookup.value(file);
	if (fileEntry && fileEntry->parent)
	{
		int row = fileEntry->parent->children.indexOf(fileEntry);
		if (row >= 0)
			emit dataChanged(createIndex(row, 0, fileEntry), createIndex(row, 1, fileEntry));
	}
}

QModelIndex OpenFileTreeModel::addToTree(QModelIndex parent, Entry* entry)
{
	Entry* parentEntry = parent.isValid() ? static_cast<Entry*>(parent.internalPointer()) : mTopLevelEntry;

	//	Work out where within the parent to insert this new item... (Maintaining alphabetic sorting)
	int insertIndex;
	for (insertIndex = 0; insertIndex < parentEntry->children.length(); insertIndex++)
		if (parentEntry->children[insertIndex]->location.getPath() > entry->location.getPath())
			break;

	//	Insert it!
	beginInsertRows(parent, insertIndex, insertIndex);
	parentEntry->children.insert(insertIndex, entry);
	entry->parent = parentEntry;
	endInsertRows();

	QModelIndex index = createIndex(insertIndex, 0, entry);
	return index;
}

QModelIndex OpenFileTreeModel::registerDirectory(const Location& location)
{
	//	First see if the directory is already in the tree...
	for (int row = 0; row < mTopLevelEntry->children.length(); row++)
		if (mTopLevelEntry->children[row]->location == location)
			return createIndex(row, 0, mTopLevelEntry->children[row]);

	//	Since it is not, add it.
	Entry* newEntry = new Entry();
	newEntry->location = location;
	return addToTree(QModelIndex(), newEntry);
}

QModelIndex OpenFileTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	Entry* parentEntry = (parent.isValid() ? static_cast<Entry*>(parent.internalPointer()) : mTopLevelEntry);
	if (parentEntry->children.length() <= row)
		return QModelIndex();
	Entry* entry = parentEntry->children[row];

	return createIndex(row, column, entry);
}

QModelIndex OpenFileTreeModel::parent(const QModelIndex& index) const
{
	Entry* entry = static_cast<Entry*>(index.internalPointer());
	if (!entry || !entry->parent || entry->parent == mTopLevelEntry)
		return QModelIndex();

	Entry* parentEntry = entry->parent;
	Entry* grandparentEntry = parentEntry->parent;

	int row = grandparentEntry->children.indexOf(parentEntry);
	return createIndex(row, 0, parentEntry);
}

int OpenFileTreeModel::rowCount(const QModelIndex& parent) const
{
	Entry* parentEntry = (parent.isValid() ? static_cast<Entry*>(parent.internalPointer()) : mTopLevelEntry);
	return parentEntry->children.length();
}

int OpenFileTreeModel::columnCount(const QModelIndex& /*parent*/) const
{
	return (mOptionFlags & OpenFileTreeView::CloseButtons ? 2 : 1);
}

QVariant OpenFileTreeModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	Entry* entry = static_cast<Entry*>(index.internalPointer());
	if (role == Qt::ToolTipRole)
	{
		QString tooltip = entry->location.getPath();
		if (entry->file)
		{
			if (entry->file->hasUnsavedChanges()) tooltip += "\nUnsaved Changes";
			tooltip += QString("\n") + entry->file->sStatusLabels[entry->file->getOpenStatus()];
		}
		return QVariant(tooltip);
	}
	else if (role < Qt::UserRole)
		 return QVariant();

	if (role == LocationRole)
		return QVariant::fromValue<Location>(entry->location);
	else
		return QVariant::fromValue<void*>(entry->file);
}

Qt::ItemFlags OpenFileTreeModel::flags(const QModelIndex &index) const
{
	Entry* entry = static_cast<Entry*>(index.internalPointer());
	return (entry->file ? Qt::ItemIsSelectable : Qt::NoItemFlags) | Qt::ItemIsEnabled;
}

QModelIndex OpenFileTreeModel::findFile(BaseFile* file) const
{
	Entry* fileEntry = mFileLookup.value(file);
	if (fileEntry && fileEntry->parent)
	{
		int row = fileEntry->parent->children.indexOf(fileEntry);
		if (row >= 0)
			return createIndex(row, 0, fileEntry);
	}

	return QModelIndex();
}

void OpenFileTreeModel::removeEntry(Entry* entry)
{
	Entry* parentEntry = entry->parent;
	int row = parentEntry->children.indexOf(entry);

	QModelIndex index = createIndex(row, 0, entry);
	QModelIndex parentIndex = parent(index);

	beginRemoveRows(parentIndex, row, row);
	parentEntry->children.removeAt(row);
	endRemoveRows();

	//	If this is a file being removed, also remove the directory above it if its now empty
	if (entry->file)
		if (parentEntry->children.length() == 0)
			removeEntry(parentEntry);
}

BaseFile* OpenFileTreeModel::getFileAtIndex(const QModelIndex& index)
{
	if (index.isValid())
	{
		Entry* entry = static_cast<Entry*>(index.internalPointer());
		if (entry)
			return entry->file;
	}

	return NULL;
}

QList<BaseFile*> OpenFileTreeModel::getIndexAndChildFiles(const QModelIndex& index)
{
	QList<BaseFile*> closingFiles;

	if (!index.isValid()) return closingFiles;

	Entry* entry = static_cast<Entry*>(index.internalPointer());
	if (!entry) return closingFiles;

	if (entry->file)
		closingFiles.append(entry->file);
	else
	{
		//	Close a whole path
		foreach (Entry* childEntry, entry->children)
			if (childEntry->file)
				closingFiles.append(childEntry->file);
	}

	return closingFiles;
}

void OpenFileTreeModel::removeFile(BaseFile* file)
{
	QModelIndex index = findFile(file);
	if (index.isValid())
	{
		Entry* entry = static_cast<Entry*>(index.internalPointer());
		removeEntry(entry);
	}

	mFiles.removeAll(file);
}










