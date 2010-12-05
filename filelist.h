#ifndef FILELIST_H
#define FILELIST_H

#include <QDockWidget>
#include <QTreeView>

class Editor;
class BaseFile;
class AutoExpandTreeView;
class OpenFileModel;

class FileList : public QDockWidget
{
    Q_OBJECT
public:
    explicit FileList(QWidget *parent = 0);

	BaseFile* getSelectedFile();

private slots:
	void selectFile(BaseFile* file);
	void fileSelected();
	void itemClicked(QModelIndex index);

private:
	AutoExpandTreeView* mTreeView;
	OpenFileModel* mFileModel;
};

#endif // FILELIST_H
