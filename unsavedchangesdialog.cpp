#include "unsavedchangesdialog.h"
#include "ui_unsavedchangesdialog.h"

#include "basefile.h"
#include <QDebug>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include "openfiletreeview.h"

UnsavedChangesDialog::UnsavedChangesDialog(const QList<BaseFile*>& files) :
	QDialog(0)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	setLayout(layout);

	QLabel* label = new QLabel(this);
	label->setText("The following files have unsaved changes: ");
	layout->addWidget(label);

	mTreeView = new OpenFileTreeView(this, OpenFileTreeView::MultiSelect | OpenFileTreeView::UnsavedOnly, &files);
	layout->addWidget(mTreeView);
	mTreeView->expandAll();
	mTreeView->setMinimumWidth(250);
	mTreeView->setMinimumHeight(200);

	mButtonBox = new QDialogButtonBox(this);
	mButtonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
	layout->addWidget(mButtonBox);

	//connect(ui->fileList, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
	//connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

UnsavedChangesDialog::~UnsavedChangesDialog()
{
}

void UnsavedChangesDialog::buttonClicked(QAbstractButton* button)
{
	/*QList<QTableWidgetItem*> items = ui->fileList->selectedItems();
	if (button == (QAbstractButton*)ui->buttonBox->button(QDialogButtonBox::Save))
	{
		//
		//	Save clicked.
		//

		QList<QTableWidgetItem*> selectedItems = ui->fileList->selectedItems();
		foreach (QTableWidgetItem* item, selectedItems)
		{
			QVariant data = item->data();
			if (data.isValid())
			{
				BaseFile* file = (BaseFile*)data.value<void*>();

			}
		}
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
	}*/
}

void UnsavedChangesDialog::selectionChanged()
{
	/*bool itemsSelected = ui->fileList->selectedItems().length() > 0;
	ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(itemsSelected);
	ui->buttonBox->button(QDialogButtonBox::Discard)->setEnabled(itemsSelected);*/
}



