#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>
#include <QTextEdit>
#include <QStackedWidget>
#include <QStatusBar>
#include <QLabel>

#include "file/location.h"

class Editor;
class FileList;
class BaseFile;
class SearchBar;
class UnsavedChangesDialog;

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
	void saveFileAs();
	void closeFile();
	void fileSelected(BaseFile* file);

	void undo();
	void redo();
	void cut();
	void copy();
	void paste();
	void selectAll();

	void showSearchBar();
	void showAdvancedSearch();

	void find(const QString& text, bool backwards);
	void find(const QString& text, bool backwards, bool caseSensitive, bool useRegexp);
	void globalFind(const QString& text, const QString& filePattern, bool backwards, bool caseSensitive, bool useRegexp);

	void replace(const QString& findText, const QString& replaceText, bool all);
	void replace(const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegexp, bool all);
	void globalReplace(const QString& findText, const QString& replaceText, const QString& filePattern, bool caseSensitive, bool useRegexp, bool all);

	void options();
	void about();

	void showErrorMessage(QString error);
	void showStatusMessage(QString message);

	void fileClosed(BaseFile* file);
	void syntaxMenuOptionClicked();

	void currentEditorChanged();
	void updateSyntaxSelection();

protected:
	void closeEvent(QCloseEvent* event);

private:
	void createToolbar();
	void createFileMenu();
	void createViewMenu();
	void createEditMenu();
	void createToolsMenu();
	void createHelpMenu();
	void createSearchBar();
	void restoreState();

	int find(Editor* editor, const QString& text, bool backwards, bool caseSensitive, bool useRegexp);
	int replace(Editor* editor, const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegexp, bool all);

	Editor* getCurrentEditor();

	FileList* mFileList;
	QStackedWidget* mEditorStack;
	QStatusBar* mStatusBar;
	QLabel* mStatusLine;
	QList<Editor*> mEditors;

	QMap<QString, QAction*> mSyntaxMenuEntries;
	QAction* mCurrentSyntaxMenuItem;
	QMenu* mSyntaxMenu;

	QDockWidget* mSearchBarWrapper;
	SearchBar* mSearchBar;
	UnsavedChangesDialog* mUnsavedChangesDialog;
};

#endif // MAINWINDOW_H
