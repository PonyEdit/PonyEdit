#ifndef FILELIST_H
#define FILELIST_H

HIDE_COMPILE_WARNINGS

#include <QDockWidget>
#include <QTreeView>

UNHIDE_COMPILE_WARNINGS

class Editor;
class BaseFile;
class OpenFileTreeView;

class FileList : public QDockWidget
{
    Q_OBJECT
public:
    explicit FileList(QWidget *parent = 0);

	FileList(FileList const&) = delete;
	FileList& operator=(FileList const&) = delete;
		
private slots:
	void selectFile(BaseFile* file);
	void fileSelected();

private:
	OpenFileTreeView* mTreeView;
};

#endif // FILELIST_H
