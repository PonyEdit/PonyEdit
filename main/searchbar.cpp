#include "searchbar.h"
#include "ui_searchbar.h"
#include <QKeyEvent>

SearchBar::SearchBar(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SearchBar)
{
	ui->setupUi(this);

	connect(ui->closeButton, SIGNAL(clicked()), this, SIGNAL(closeRequested()));
	connect(ui->prevButton, SIGNAL(clicked()), this, SLOT(findPrev()));
	connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(findNext()));
}

SearchBar::~SearchBar()
{
    delete ui;
}

void SearchBar::takeFocus()
{
	ui->find->setFocus();
	ui->find->selectAll();
}

void SearchBar::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_Enter:
	case Qt::Key_Return:
		findNext();
		break;

	case Qt::Key_Escape:
		emit closeRequested();
		break;
	}
}

void SearchBar::findNext()
{
	emit find(ui->find->text(), false);
}

void SearchBar::findPrev()
{
	emit find(ui->find->text(), true);
}

