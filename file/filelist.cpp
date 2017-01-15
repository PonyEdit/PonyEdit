HIDE_COMPILE_WARNINGS

#include <QVBoxLayout>
#include <QDebug>
#include <QMap>

UNHIDE_COMPILE_WARNINGS

#include "file/openfiletreeview.h"
#include "main/globaldispatcher.h"
#include "file/basefile.h"
#include "file/filelist.h"
#include "editor/editor.h"

FileList::FileList(QWidget *parent) :
    QDockWidget(parent),
    mTreeView(new OpenFileTreeView(this, OpenFileTreeView::CloseButtons | OpenFileTreeView::RefreshButtons))
{
	setWindowTitle("Open Files");
	QWidget* titleWidget = new QWidget(this);
	setTitleBarWidget( titleWidget );

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













