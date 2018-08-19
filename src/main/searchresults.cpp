#include <QDebug>
#include <QHBoxLayout>
#include <QTextBlock>
#include <QVBoxLayout>

#include "file/basefile.h"
#include "searchresultdelegate.h"
#include "searchresultmodel.h"
#include "searchresults.h"
#include "windowmanager.h"

SearchResults::SearchResults( QWidget *parent ) :
	QWidget( parent ) {
	auto *mainLayout = new QVBoxLayout( this );
	mainLayout->setMargin( 0 );

	mModel    = new SearchResultModel( this );
	mDelegate = new SearchResultDelegate( mModel, this );

	mTreeView = new QTreeView( this );
	mTreeView->setModel( mModel );
	mTreeView->setItemDelegate( mDelegate );
	mTreeView->setHeaderHidden( true );
	mainLayout->addWidget( mTreeView );

	auto *childLayout = new QHBoxLayout();
	mainLayout->addLayout( childLayout );
	childLayout->setMargin( 5 );

	mReplaceLabel = new QLabel( tr( "Replace With: " ), this );
	mReplaceLabel->setMargin( 5 );
	childLayout->addWidget( mReplaceLabel );

	mReplaceWithText = new QLineEdit( this );
	mReplaceWithText->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
	childLayout->addWidget( mReplaceWithText );

	mReplaceButton = new QPushButton( tr( "Replace" ), this );
	mReplaceLabel->setMargin( 5 );
	childLayout->addWidget( mReplaceButton );

	connect( mTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)) );
	connect( mReplaceButton, SIGNAL(clicked()), this, SLOT(replaceClicked()) );
}

void SearchResults::clearResults() {
	mModel->clear();
}

void SearchResults::showResults( const QList< SearchResultModel::Result > &results ) {
	mModel->clear();
	mModel->addResults( results );
}

void SearchResults::itemClicked( QModelIndex index ) {
	SearchResultModel::Result *result = mModel->getResultForIndex( index );
	if ( result != nullptr ) {
		gWindowManager->showAndSelect( result->location, result->lineNumber, result->start, result->length );
	}
}

void SearchResults::showReplaceOptions( bool replaceOptions ) {
	mModel->setShowCheckboxes( replaceOptions );
	mReplaceLabel->setVisible( replaceOptions );
	mReplaceWithText->setVisible( replaceOptions );
	mReplaceButton->setVisible( replaceOptions );
}

void SearchResults::replaceClicked() {
	mModel->replaceSelectedResults( mReplaceWithText->text() );
	gWindowManager->hideSearchResults();
}
