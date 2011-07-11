#include "searchresults.h"
#include <QVBoxLayout>
#include "file/basefile.h"
#include <QTextBlock>
#include <QDebug>
#include "searchresultmodel.h"
#include "searchresultdelegate.h"
#include "windowmanager.h"

SearchResults::SearchResults(QWidget *parent) :
	QWidget(parent)
{
	mLayout = new QVBoxLayout(this);
	mLayout->setMargin(0);

	mModel = new SearchResultModel(this);
	mDelegate = new SearchResultDelegate(mModel, this);

	mTreeView = new QTreeView(this);
	mTreeView->setModel(mModel);
	mTreeView->setItemDelegate(mDelegate);
	mTreeView->setHeaderHidden(true);

	mLayout->addWidget(mTreeView);

	connect(mTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
}

void SearchResults::clearResults()
{
	mModel->clear();
}

void SearchResults::showResults(const QList<SearchResultModel::Result>& results)
{
	mModel->clear();
	mModel->addResults(results);
}

void SearchResults::itemClicked(QModelIndex index)
{
	SearchResultModel::Result* result = mModel->getResultForIndex(index);
	if (result != NULL)
		gWindowManager->showAndSelect(result->location, result->lineNumber, result->start, result->length);
}


