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

private slots:
	void selectionChanged(QListWidgetItem * current, QListWidgetItem * previous);
	void activeFileListUpdated();

signals:
	void fileSelected(BaseFile* file);

private:
	QListWidget* mListWidget;
};

#endif // FILELIST_H
