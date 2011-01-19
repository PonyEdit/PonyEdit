#include <QKeyEvent>

#include "advancedsearchdialog.h"
#include "ui_advancedsearchdialog.h"

AdvancedSearchDialog::AdvancedSearchDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdvancedSearchDialog)
{
    ui->setupUi(this);

	ui->context->addItem(tr("All Files"));
	ui->context->addItem(tr("Current File"));

	ui->caseSensitive->setChecked(true);

	ui->filePattern->setText("*");

	ui->find->setFocus();

	connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(findNext()));
	connect(ui->previousButton, SIGNAL(clicked()), this, SLOT(findPrevious()));
	connect(ui->replaceButton, SIGNAL(clicked()), this, SLOT(replaceCurrent()));
	connect(ui->replaceFindButton, SIGNAL(clicked()), this, SLOT(replaceCurrentAndFind()));
	connect(ui->replaceAllButton, SIGNAL(clicked()), this, SLOT(replaceAll()));
	connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

AdvancedSearchDialog::~AdvancedSearchDialog()
{
    delete ui;
}

void AdvancedSearchDialog::findNext()
{
	if(ui->context->currentIndex() == 0)
		emit globalFind(ui->find->text(), ui->filePattern->text(), false, ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked());
	else
		emit find(ui->find->text(), false, ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked());
}

void AdvancedSearchDialog::findPrevious()
{
	if(ui->context->currentIndex() == 0)
		emit globalFind(ui->find->text(), ui->filePattern->text(), true, ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked());
	else
		emit find(ui->find->text(), true, ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked());

}

void AdvancedSearchDialog::replaceCurrent()
{
	if(ui->context->currentIndex() == 0)
		emit globalReplace(ui->find->text(), ui->replace->text(), ui->filePattern->text(), ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked(), false);
	else
		emit replace(ui->find->text(), ui->filePattern->text(), ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked(), false);
}

void AdvancedSearchDialog::replaceCurrentAndFind()
{
	replaceCurrent();
	findNext();
}

void AdvancedSearchDialog::replaceAll()
{
	if(ui->context->currentIndex() == 0)
		emit globalReplace(ui->find->text(), ui->replace->text(), ui->filePattern->text(), ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked(), true);
	else
		emit replace(ui->find->text(), ui->filePattern->text(), ui->caseSensitive->isChecked(), ui->regularExpressions->isChecked(), true);
}

void AdvancedSearchDialog::keyPressEvent(QKeyEvent *event)
{
	if(event->matches(QKeySequence::FindNext))
		findNext();
	else if(event->matches(QKeySequence::FindPrevious))
		findPrevious();
	else if(event->key() == Qt::Key_Escape)
		close();
}
