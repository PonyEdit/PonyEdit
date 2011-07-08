#ifndef SEARCHRESULTS_H
#define SEARCHRESULTS_H

#include <QTreeWidget>

class SearchResults : public QTreeWidget
{
    Q_OBJECT
public:
    explicit SearchResults(QWidget *parent = 0);
};

#endif // SEARCHRESULTS_H
