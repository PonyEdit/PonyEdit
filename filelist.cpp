#include "file.h"
#include "filelist.h"
#include <QVBoxLayout>

FileList::FileList(QWidget *parent) :
    QDockWidget(parent)
{
	setWindowTitle("Open Files");

	mListWidget = new QListWidget();
	mListWidget->setMinimumWidth(150);
	setWidget(mListWidget);
}

void FileList::update(const QList<Editor*>& list)
{
	mListWidget->clear();

	foreach (Editor* editor, list)
	{
		Location location = editor->getLocation();

		QListWidgetItem* item = new QListWidgetItem();
		item->setIcon(location.getIcon());
		item->setText(location.getLabel());
		mListWidget->addItem(item);
	}
}
