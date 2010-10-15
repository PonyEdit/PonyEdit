#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QTextEdit>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void docChanged(int, int, int);

private:
	QTextDocument* mCurrentDocument;
	QTextEdit* mEditor;
};

#endif // MAINWINDOW_H
