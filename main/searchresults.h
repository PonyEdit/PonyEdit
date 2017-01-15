#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H

HIDE_COMPILE_WARNINGS

#include <QTreeView>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

UNHIDE_COMPILE_WARNINGS

#include "searchresultmodel.h"

class SearchResultDelegate;

class SearchResults : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResults(QWidget *parent = 0);

	SearchResults(SearchResults const&) = delete;
	SearchResults& operator=(SearchResults const&) = delete;
	
	void clearResults();
	void showResults(const QList<SearchResultModel::Result>& results);

	void showReplaceOptions(bool replaceOptions);

private slots:
	void itemClicked(QModelIndex index);
	void replaceClicked();

private:
	QTreeView* mTreeView;

	QLabel* mReplaceLabel;
	QLineEdit* mReplaceWithText;
	QPushButton* mReplaceButton;

	SearchResultModel* mModel;
	SearchResultDelegate* mDelegate;
};

#endif // SEARCHRESULTS_H
