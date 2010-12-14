#include <QVBoxLayout>
#include <QDebug>
#include <QMap>

#include "file/openfiletreeview.h"
#include "main/globaldispatcher.h"
#include "file/basefile.h"
#include "file/filelist.h"
#include "editor/editor.h"

FileList::FileList(QWidget *parent) :
    QDockWidget(parent)
{
	setWindowTitle("Open Files");

	mTreeView = new OpenFileTreeView(this, OpenFileTreeView::CloseButtons);
	mTreeView->setMinimumWidth(150);
	setWidget(mTreeView);

	connect(gDispatcher, SIGNAL(selectFile(BaseFile*)), this, SLOT(selectFile(BaseFile*)));
	connect(mTreeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(fileSelected()));
}

void FileList::selectFile(BaseFile* file)
{
	mTreeView->selectFile(file);
}

void FileList::fileSelected()
{
	BaseFile* file = mTreeView->getSelectedFile();
	gDispatcher->emitSelectFile(file);
}













