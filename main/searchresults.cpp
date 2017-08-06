HIDE_COMPILE_WARNINGS

#include <QTextBlock>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>

UNHIDE_COMPILE_WARNINGS

#include "searchresults.h"
#include "file/basefile.h"
#include "searchresultmodel.h"
#include "searchresultdelegate.h"
#include "windowmanager.h"

SearchResults::SearchResults(QWidget *parent) : QWidget(parent),
    mTreeView(new QTreeView(this)),
    mReplaceLabel(new QLabel(tr("Replace With: "), this)),
    mReplaceWithText(new QLineEdit(this)),
    mReplaceButton(new QPushButton(tr("Replace"), this)),
    mModel(new SearchResultModel(this)),
    mDelegate(new SearchResultDelegate(mModel, this))
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);

	mTreeView->setModel(mModel);
	mTreeView->setItemDelegate(mDelegate);
	mTreeView->setHeaderHidden(true);
	mainLayout->addWidget(mTreeView);

	QHBoxLayout* childLayout = new QHBoxLayout();
	mainLayout->addLayout(childLayout);
	childLayout->setMargin(5);

	mReplaceLabel->setMargin(5);
	childLayout->addWidget(mReplaceLabel);

	mReplaceWithText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	childLayout->addWidget(mReplaceWithText);

	mReplaceLabel->setMargin(5);
	childLayout->addWidget(mReplaceButton);

	connect(mTreeView, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
	connect(mReplaceButton, SIGNAL(clicked()), this, SLOT(replaceClicked()));
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

void SearchResults::showReplaceOptions(bool replaceOptions)
{
	mModel->setShowCheckboxes(replaceOptions);
	mReplaceLabel->setVisible(replaceOptions);
	mReplaceWithText->setVisible(replaceOptions);
	mReplaceButton->setVisible(replaceOptions);
}

void SearchResults::replaceClicked()
{
	mModel->replaceSelectedResults(mReplaceWithText->text());
	gWindowManager->hideSearchResults();
}


