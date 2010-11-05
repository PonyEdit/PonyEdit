#include <QVBoxLayout>

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
		item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)editor));
		mListWidget->addItem(item);
	}
}

void FileList::selectionChanged(QListWidgetItem* current, QListWidgetItem*)
{
	if (!current) return;
	Editor* editor = (Editor*)current->data(Qt::UserRole).value<void*>();
	emit fileSelected(editor);
}
