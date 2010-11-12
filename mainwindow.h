#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>
#include <QTextEdit>
#include <QStackedWidget>
#include <QStatusBar>
#include <QLabel>

class Editor;
class FileList;
class BaseFile;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

	void createFileMenu();
	void createToolsMenu();
	void createHelpMenu();

public slots:
	void newFile();
	void openFile();
	void saveFile();
	void fileSelected(BaseFile* file);

	void options();
	void about();

	void showErrorMessage(QString error);
	void showStatusMessage(QString message);

private:
	void createToolbar();

	FileList* mFileList;
	QStackedWidget* mEditorStack;
	QStatusBar* mStatusBar;
	QLabel* mStatusLine;
	QList<Editor*> mEditors;
};

#endif // MAINWINDOW_H
