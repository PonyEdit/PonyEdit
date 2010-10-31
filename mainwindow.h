#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>
#include <QTextEdit>
#include <QStackedWidget>

#include "sshconnection.h"
#include "sshremotecontroller.h"
#include "serverconfigdlg.h"
#include "editor.h"

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

private:
	void createToolbar();

	QStackedWidget* mEditorStack;
	QList<Editor*> mEditors;
};

#endif // MAINWINDOW_H
