#include <QVBoxLayout>
#include <QMap>

#include "globaldispatcher.h"
#include "basefile.h"
#include "filelist.h"
#include "editor.h"

FileList::FileList(QWidget *parent) :
    QDockWidget(parent)
{
	setWindowTitle("Open Files");

	mListWidget = new QListWidget();
	mListWidget->setMinimumWidth(150);
	setWidget(mListWidget);

	connect(mListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(selectionChanged(QListWidgetItem*,QListWidgetItem*)));
	connect(gDispatcher, SIGNAL(activeFilesUpdated()), this, SLOT(activeFileListUpdated()));
}

void FileList::selectionChanged(QListWidgetItem* current, QListWidgetItem*)
{
	/*if (!current) return;
	Editor* editor = (Editor*)current->data(Qt::UserRole).value<void*>();
	emit fileSelected(editor);*/
}

void FileList::activeFileListUpdated()
{
	//	Take a quick inventory of the files in the list now...
	QMap<BaseFile*, bool> currentList;
	for (int row = 0; row < mListWidget->count(); row++)
	{
		QListWidgetItem* item = mListWidget->item(row);
		BaseFile* file = (BaseFile*)item->data(Qt::UserRole).value<void*>();
		currentList.insert(file, false);
	}

	//	Go through the list of files that should be there. Add new entries, mark existing ones as "ok to keep"
	const QList<BaseFile*>& fileList = BaseFile::getActiveFiles();
	foreach (BaseFile* file, fileList)
	{
		if (currentList.contains(file))
			currentList.insert(file, true);
		else
		{
			Location location = file->getLocation();

			QListWidgetItem* item = new QListWidgetItem();
			item->setIcon(location.getIcon());
			item->setText(location.getLabel());
			item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)file));
			mListWidget->addItem(item);
		}
	}

	//	Remove the list entries that have not been marked as "ok to keep"
	for (int row = 0; row < mListWidget->count(); row++)
	{
		QListWidgetItem* item = mListWidget->item(row);
		BaseFile* file = (BaseFile*)item->data(Qt::UserRole).value<void*>();
		if (!currentList.value(file, true))
		{
			row--;
			delete item;
		}
	}
}







