#ifndef FILELIST_H
#define FILELIST_H

#include <QDockWidget>
#include <QListWidget>
#include <QList>
#include "editor.h"

class FileList : public QDockWidget
{
    Q_OBJECT
public:
    explicit FileList(QWidget *parent = 0);

	void update(const QList<Editor*>& list);

private:
	QListWidget* mListWidget;
};

#endif // FILELIST_H
