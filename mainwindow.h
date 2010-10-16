#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>
#include <QTextEdit>

#include "sshconnection.h"
#include "sshremotecontroller.h"
#include "serverconfigdlg.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void docChanged(int, int, int);

	void newFile();
	void openFile();
	void saveFile();

private:
	void createToolbar();

	SshRemoteController* mController;
	QTextDocument* mCurrentDocument;
	QTextEdit* mEditor;
};

#endif // MAINWINDOW_H
