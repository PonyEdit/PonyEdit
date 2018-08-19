#include <QKeyEvent>

#include "searchbar.h"
#include "ui_searchbar.h"

SearchBar::SearchBar( QWidget *parent ) :
	QWidget( parent ),
	ui( new Ui::SearchBar ) {
	ui->setupUi( this );

#ifdef Q_OS_MAC
	ui->replaceButton->setMinimumHeight( 32 );
	ui->replaceFindButton->setMinimumHeight( 32 );
	ui->replaceAllButton->setMinimumHeight( 32 );
#endif

	connect( ui->closeButton, SIGNAL(clicked()), this, SIGNAL(closeRequested()) );
	connect( ui->prevButton, SIGNAL(clicked()), this, SLOT(findPrev()) );
	connect( ui->nextButton, SIGNAL(clicked()), this, SLOT(findNext()) );
	connect( ui->replaceButton, SIGNAL(clicked()), this, SLOT(replaceCurrent()) );
	connect( ui->replaceFindButton, SIGNAL(clicked()), this, SLOT(replaceCurrentAndFind()) );
	connect( ui->replaceAllButton, SIGNAL(clicked()), this, SLOT(replaceAll()) );
}

SearchBar::~SearchBar() {
	delete ui;
}

void SearchBar::takeFocus() {
	ui->find->setFocus();
	ui->find->selectAll();
}

void SearchBar::keyPressEvent( QKeyEvent *event ) {
	switch ( event->key() ) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			findNext();
			break;

		case Qt::Key_Escape:
			emit closeRequested();
			break;
	}
}

void SearchBar::findNext() {
	emit find( ui->find->text(), false );
}

void SearchBar::findPrev() {
	emit find( ui->find->text(), true );
}

void SearchBar::replaceCurrent() {
	emit replace( ui->find->text(), ui->replace->text(), false );
}

void SearchBar::replaceCurrentAndFind() {
	replaceCurrent();
	findNext();
}

void SearchBar::replaceAll() {
	emit replace( ui->find->text(), ui->replace->text(), true );
}
