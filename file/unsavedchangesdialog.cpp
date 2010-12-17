#include "unsavedchangesdialog.h"

#include "basefile.h"
#include <QDebug>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include "openfiletreeview.h"
#include "openfilemanager.h"

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
	mTreeView->selectAll();

	mButtonBox = new QDialogButtonBox(this);
	mButtonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Discard | QDialogButtonBox::Cancel);
	layout->addWidget(mButtonBox);

	foreach (BaseFile* file, files)
		connect(file, SIGNAL(unsavedStatusChanged()), this, SLOT(fileStateChanged()));

	connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged(QItemSelection,QItemSelection)));
	connect(mButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(mButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
	connect(&gOpenFileManager, SIGNAL(fileClosed(BaseFile*)), this, SLOT(fileClosed(BaseFile*)));
}

UnsavedChangesDialog::~UnsavedChangesDialog()
{
}

void UnsavedChangesDialog::buttonClicked(QAbstractButton* button)
{
	QList<BaseFile*> selectedFiles = mTreeView->getSelectedFiles();
	if (button == (QAbstractButton*)mButtonBox->button(QDialogButtonBox::Save))
	{
		//	Save
		foreach (BaseFile* file, selectedFiles)
			file->save();
	}
	else if (button == (QAbstractButton*)mButtonBox->button(QDialogButtonBox::Discard))
	{
		//	Discard
		foreach (BaseFile* file, selectedFiles)
			file->close();
	}
}

void UnsavedChangesDialog::selectionChanged(QItemSelection /* before */, QItemSelection /* after */)
{
	bool itemsSelected = mTreeView->selectionModel()->selectedRows().count() > 0;
	mButtonBox->button(QDialogButtonBox::Save)->setEnabled(itemsSelected);
	mButtonBox->button(QDialogButtonBox::Discard)->setEnabled(itemsSelected);
}

void UnsavedChangesDialog::fileStateChanged()
{
	BaseFile* file = static_cast<BaseFile*>(QObject::sender());
	if (!file->hasUnsavedChanges())
	{
		BaseFile::OpenStatus status = file->getOpenStatus();

		qDebug() << "Status = " << status << " closed = " << BaseFile::Closed;

		if (status != BaseFile::Closed && status != BaseFile::Closing)
			file->close();
	}
}

void UnsavedChangesDialog::fileClosed(BaseFile* file)
{
	mTreeView->removeFile(file);
	if (mTreeView->model()->rowCount() == 0)
		accept();
}
