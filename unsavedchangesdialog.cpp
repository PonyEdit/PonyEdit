#include "unsavedchangesdialog.h"
#include "ui_unsavedchangesdialog.h"

#include "basefile.h"
#include <QDebug>
#include <QPushButton>
#include <QTreeWidgetItem>

UnsavedChangesDialog::UnsavedChangesDialog(QList<BaseFile*> files) :
	QDialog(0),
    ui(new Ui::UnsavedChangesDialog)
{
    ui->setupUi(this);

	//
	//	Configure the shape and size of the table...
	//

	ui->fileList->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
	ui->fileList->horizontalHeader()->setResizeMode(1, QHeaderView::Fixed);
	ui->fileList->setColumnWidth(1, 80);
	ui->fileList->setRowCount(files.count());
	int lineHeight = ui->fileList->fontMetrics().height() + 2;

	//
	//	Populate the table
	//

	QTableWidgetItem* item;
	mFiles = files;
	int row = 0;
	foreach (BaseFile* file, files)
	{
		Location fileLocation = file->getLocation();

		//	Column 1: Full path of the file, elided on the left
		item = new QTableWidgetItem(fileLocation.getIcon(), fileLocation.getPath());
		item->setData(Qt::UserRole, QVariant::fromValue<void*>(file));
		ui->fileList->setItem(row, 0, item);

		//	Column 2: Status of the file...
		item = new QTableWidgetItem("Unsaved");
		ui->fileList->setItem(row, 1, item);

		ui->fileList->setRowHeight(row, lineHeight);
		row++;
	}

	ui->fileList->selectAll();

	connect(ui->fileList, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

UnsavedChangesDialog::~UnsavedChangesDialog()
{
    delete ui;
}

void UnsavedChangesDialog::buttonClicked(QAbstractButton* button)
{
	QList<QTableWidgetItem*> items = ui->fileList->selectedItems();
	if (button == (QAbstractButton*)ui->buttonBox->button(QDialogButtonBox::Save))
	{
		//
		//	Save clicked.
		//

		/*QList<QTableWidgetItem*> selectedItems = ui->fileList->selectedItems();
		foreach (QTableWidgetItem* item, selectedItems)
		{
			QVariant data = item->data();
			if (data.isValid())
			{
				BaseFile* file = (BaseFile*)data.value<void*>();

			}
		}*/
	}
	else if (button == (QAbstractButton*)ui->buttonBox->button(QDialogButtonBox::Discard))
	{
		//
		//	Discard clicked. Just need to remove from from the list.
		//

		QList<int> removeRows;
		foreach (QTableWidgetItem* item, items)
		{
			qDebug() << item->row();
		}
	}
}

void UnsavedChangesDialog::selectionChanged()
{
	bool itemsSelected = ui->fileList->selectedItems().length() > 0;
	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(itemsSelected);
	ui->buttonBox->button(QDialogButtonBox::Discard)->setEnabled(itemsSelected);
}



