#include <QKeyEvent>

#include "advancedsearchdialog.h"
#include "ui_advancedsearchdialog.h"
#include "windowmanager.h"
#include "file/basefile.h"
#include "file/openfilemanager.h"

AdvancedSearchDialog::AdvancedSearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvancedSearchDialog)
{
    ui->setupUi(this);

	ui->context->addItem(tr("All Files"), QVariant(OpenFiles));
	ui->context->addItem(tr("Current File"), QVariant(CurrentFile));

	ui->caseSensitive->setChecked(true);
	ui->filePattern->setText("*");

	ui->find->setFocus();

	connect(ui->searchButton, SIGNAL(clicked()), this, SLOT(search()));
	connect(ui->searchReplaceButton, SIGNAL(clicked()), this, SLOT(searchAndReplace()));
	connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

AdvancedSearchDialog::~AdvancedSearchDialog()
{
    delete ui;
}

void AdvancedSearchDialog::search()
{
	gWindowManager->searchInFiles(getLocalHaystackFiles(), ui->find->text(), ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked(), false);
	close();
}

void AdvancedSearchDialog::searchAndReplace()
{
	gWindowManager->searchInFiles(getLocalHaystackFiles(), ui->find->text(), ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked(), true);
	close();
}

QList<BaseFile*> AdvancedSearchDialog::getLocalHaystackFiles()
{
	QList<BaseFile*> result;
	Scope scope = static_cast<Scope>(ui->context->itemData(ui->context->currentIndex()).toInt());

	if (scope == CurrentFile)
	{
		if (gWindowManager->getCurrentFile() != NULL)
			result.append(gWindowManager->getCurrentFile());
	}
	else if (scope == OpenFiles)
	{
		QRegExp namePattern(ui->filePattern->text(), Qt::CaseInsensitive, QRegExp::WildcardUnix);
		QList<BaseFile*> files = gOpenFileManager.getOpenFiles();
		foreach (BaseFile* file, files)
			if (namePattern.exactMatch(file->getLocation().getPath()))
				result.append(file);
	}

	return result;
}










