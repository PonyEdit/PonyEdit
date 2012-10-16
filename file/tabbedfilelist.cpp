#include <QDebug>
#include "openfilemanager.h"
#include "main/globaldispatcher.h"
#include "basefile.h"
#include "tabbedfilelist.h"

TabbedFileList::TabbedFileList(QWidget *parent) :
    QDockWidget(parent)
{
	setWindowTitle(tr("Tabbed File List"));
	QWidget* titleWidget = new QWidget(this);
	setTitleBarWidget( titleWidget );

	mTabs = new QTabBar(this);
	mTabs->setTabsClosable(true);

	connect(mTabs, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));
	connect(mTabs, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequested(int)));

	setFeatures(QDockWidget::NoDockWidgetFeatures);
	setWidget(mTabs);

	connect(&gOpenFileManager, SIGNAL(fileOpened(BaseFile*)), this, SLOT(fileOpened(BaseFile*)));
	connect(&gOpenFileManager, SIGNAL(fileClosed(BaseFile*)), this, SLOT(fileClosed(BaseFile*)), Qt::DirectConnection);
	connect(gDispatcher, SIGNAL(selectFile(BaseFile*)), this, SLOT(fileSelected(BaseFile*)));

	//	Add any already-open files
	foreach (BaseFile* file, gOpenFileManager.getOpenFiles())
		fileOpened(file);
}

int TabbedFileList::findTab(BaseFile *file)
{
	if(NULL == file)
		return -1;

	for(int ii = 0; ii < mTabs->count(); ii++)
	{
		if(file->getLocation() == mTabs->tabData(ii).value<Location>())
			return ii;
	}

	return -1;
}

void TabbedFileList::fileOpened(BaseFile* file)
{
	int idx = mTabs->addTab(file->getLocation().getLabel());
	mTabs->setTabData(idx, QVariant::fromValue<Location>(file->getLocation()));

	connect(file, SIGNAL(openStatusChanged(int)), this, SLOT(fileChanged()));
	connect(file, SIGNAL(fileProgress(int)), this, SLOT(fileChanged()));
	connect(file, SIGNAL(unsavedStatusChanged()), this, SLOT(fileChanged()));
}

void TabbedFileList::fileClosed(BaseFile* file)
{
	int idx = findTab(file);
	mTabs->removeTab(idx);
}

void TabbedFileList::fileSelected(BaseFile *file)
{
	int idx = findTab(file);
	mTabs->setCurrentIndex(idx);
}

void TabbedFileList::fileChanged()
{
	BaseFile* file = (BaseFile*)QObject::sender();
	int idx = findTab(file);
	if(file->hasUnsavedChanges())
		mTabs->setTabText(idx, file->getLocation().getLabel() + " *");
	else
		mTabs->setTabText(idx, file->getLocation().getLabel());
}

void TabbedFileList::currentChanged(int index)
{
	Location loc = mTabs->tabData(index).value<Location>();
	if(loc.isNull())
		return;
	gDispatcher->emitSelectFile(loc.getFile());
}

void TabbedFileList::tabCloseRequested(int index)
{
	QList<BaseFile*> closingFiles;
	Location loc = mTabs->tabData(index).value<Location>();
	if(loc.isNull())
		return;
	closingFiles.append(loc.getFile());

	gOpenFileManager.closeFiles(closingFiles);
}
