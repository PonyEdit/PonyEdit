#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>
#include <QTextEdit>
#include <QStackedWidget>

class Editor;
class FileList;
class BaseFile;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void newFile();
	void openFile();
	void saveFile();
	void fileSelected(BaseFile* file);

private:
	void createToolbar();

	FileList* mFileList;
	QStackedWidget* mEditorStack;
	QList<Editor*> mEditors;
};

#endif // MAINWINDOW_H
