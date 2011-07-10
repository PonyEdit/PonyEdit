#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H

#include <QTreeView>
#include "searchresultmodel.h"
class SearchResultDelegate;

class SearchResults : public QWidget
{
    Q_OBJECT
public:
    explicit SearchResults(QWidget *parent = 0);
	void clearResults();
	void showResults(const QList<SearchResultModel::Result>& results);

private:
	QLayout* mLayout;
	QTreeView* mTreeView;
	SearchResultModel* mModel;
	SearchResultDelegate* mDelegate;
};

#endif // SEARCHRESULTS_H
