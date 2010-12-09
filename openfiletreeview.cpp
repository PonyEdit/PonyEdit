#include "openfiletreeview.h"
#include "openfiletreemodel.h"
#include "openfileitemdelegate.h"
#include "openfilemanager.h"
#include <QHeaderView>
#include <QDebug>

OpenFileTreeView::OpenFileTreeView(QWidget *parent, int optionFlags, const QList<BaseFile*>* files) :
    QTreeView(parent)
{
	//	Create & attach the model; the model supplies the data in tree layout to display
	mModel = new OpenFileTreeModel(this, optionFlags, files);
	setModel(mModel);

	//	Create & attach the item delegate; it handles drawing each item
	mDelegate = new OpenFileItemDelegate(this);
	setItemDelegate(mDelegate);

	//	Configure look & feel details
	header()->hide();
	setAttribute(Qt::WA_MacShowFocusRect, false);
	setSelectionMode(QAbstractItemView::SingleSelection);
	viewport()->setAttribute(Qt::WA_Hover);
	header()->setResizeMode(0, QHeaderView::Stretch);

	if (optionFlags & CloseButtons)
	{
		header()->setStretchLastSection(false);
		header()->setResizeMode(1, QHeaderView::Fixed);
		header()->resizeSection(1, 16);

		connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
	}
	else
		header()->setStretchLastSection(true);
}

void OpenFileTreeView::dataChanged(const QModelIndex& /*topLeft*/, const QModelIndex& /*bottomRight*/)
{
	this->expandAll();
}

BaseFile* OpenFileTreeView::getSelectedFile() const
{
	QModelIndex i = this->selectionModel()->currentIndex();
	return mModel->getFileAtIndex(i);
}

void OpenFileTreeView::selectFile(BaseFile* file)
{
	if (!file) return;
	if (getSelectedFile() != file)
	{
		QModelIndex index = mModel->findFile(file);
		setCurrentIndex(index);
	}
}

void OpenFileTreeView::itemClicked(QModelIndex index)
{
	if (!index.isValid()) return;

	if (index.column() == 1)
	{
		//	Close button clicked.
		QList<BaseFile*> closingFiles = mModel->getIndexAndChildFiles(index);
		gOpenFileManager.closeFiles(closingFiles);
	}
}

/*
void OpenFileTreeModel::closeFiles(const QList<BaseFile*>& files)
{
	//	Look for unsaved changes
	QList<BaseFile*> unsavedFiles;
	foreach (BaseFile* file, files)
		if (file->hasUnsavedChanges())
			unsavedFiles.append(file);

	if (unsavedFiles.length())
	{
		UnsavedChangesDialog dialog(unsavedFiles);
		dialog.exec();
	}
}
*/




