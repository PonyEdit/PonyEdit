#ifndef FILELIST_H
#define FILELIST_H

#include <QDockWidget>
#include <QTreeView>

class Editor;
class BaseFile;
class OpenFileTreeView;

class FileList : public QDockWidget
{
Q_OBJECT
public:
explicit FileList( QWidget *parent = 0 );

private slots:
void selectFile( BaseFile* file );
void fileSelected();

private:
OpenFileTreeView* mTreeView;
};

#endif	// FILELIST_H
