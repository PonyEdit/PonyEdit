#ifndef FILELIST_H
#define FILELIST_H

#include <QDockWidget>
#include <QListWidget>
#include <QList>

class Editor;

class FileList : public QDockWidget
{
    Q_OBJECT
public:
    explicit FileList(QWidget *parent = 0);

	void update(const QList<Editor*>& list);

private slots:
	void selectionChanged(QListWidgetItem * current, QListWidgetItem * previous);

signals:
	void fileSelected(Editor* editor);

private:
	QListWidget* mListWidget;
};

#endif // FILELIST_H
