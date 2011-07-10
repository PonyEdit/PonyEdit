#include "searchresults.h"
#include <QVBoxLayout>
#include "file/basefile.h"
#include <QTextBlock>
#include <QDebug>
#include "searchresultmodel.h"
#include "searchresultdelegate.h"

SearchResults::SearchResults(QWidget *parent) :
	QWidget(parent)
{
	mLayout = new QVBoxLayout(this);

	mModel = new SearchResultModel(this);
	//mDelegate = new SearchResultDelegate(this);

	mTreeView = new QTreeView(this);
	mTreeView->setModel(mModel);
	//mTreeView->setItemDelegate(mDelegate);

	mLayout->addWidget(mTreeView);
}

void SearchResults::clearResults()
{
	mModel->clear();
}

void SearchResults::showResults(const QList<SearchResultModel::Result>& results)
{
	mModel->addResults(results);
}


