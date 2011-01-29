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
class ConnectionStatusPane;

#ifdef Q_OS_MAC
extern void qt_mac_set_dock_menu(QMenu *menu);
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
	void newFile();
	void openFile();
	void openSingleFile(Location* loc = NULL);
	void saveFile();
	void saveFileAs();
	void closeFile();
	void fileSelected(BaseFile* file);

	void print();

	void updateTitle();
	void updateTitle(BaseFile* file);

	void undo();
	void redo();
	void cut();
	void copy();
	void paste();
	void selectAll();

	void showGotoLine();

	void showSearchBar();
	void showAdvancedSearch();

	void find(const QString& text, bool backwards);
	void find(const QString& text, bool backwards, bool caseSensitive, bool useRegexp);
	void globalFind(const QString& text, const QString& filePattern, bool backwards, bool caseSensitive, bool useRegexp);

	void replace(const QString& findText, const QString& replaceText, bool all);
	void replace(const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegexp, bool all);
	void globalReplace(const QString& findText, const QString& replaceText, const QString& filePattern, bool caseSensitive, bool useRegexp, bool all);

	void resetZoom();
	void zoomIn();
	void zoomOut();

	void toggleFullScreen();

	void previousWindow();
	void nextWindow();

	void showHTMLPreview();
	void options();

	void about();
	void checkForUpdates();

	void showErrorMessage(QString error);
	void showStatusMessage(QString message);

	void fileClosed(BaseFile* file);
	void syntaxMenuOptionClicked();

	void currentEditorChanged();
	void updateSyntaxSelection();

	void saveFailed(const QString& error);

	Editor* getCurrentEditor();

protected:
	void closeEvent(QCloseEvent* event);
	void dragEnterEvent(QDragEnterEvent *);
	void dropEvent(QDropEvent *);

private:
	void createToolbar();
	void createFileMenu();
	void createViewMenu();
	void createEditMenu();
	void createToolsMenu();
	void createWindowMenu();
	void createHelpMenu();

	void createMacDockMenu();

	void createSearchBar();
	void restoreState();

	void updateRecentFilesMenu();
	void addRecentFile(Location* loc);

	int find(Editor* editor, const QString& text, bool backwards, bool caseSensitive, bool useRegexp, bool loop = true);
	int replace(Editor* editor, const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegexp, bool all);

	FileList* mFileList;
	QStackedWidget* mEditorStack;
	QStatusBar* mStatusBar;
	QLabel* mStatusLine;
	QList<Editor*> mEditors;

	QList<Location*> mRecentFiles;
	QMenu* mRecentFilesMenu;

	QMap<QString, QAction*> mSyntaxMenuEntries;
	QAction* mCurrentSyntaxMenuItem;
	QMenu* mSyntaxMenu;

	QDockWidget* mSearchBarWrapper;
	SearchBar* mSearchBar;
	UnsavedChangesDialog* mUnsavedChangesDialog;

	ConnectionStatusPane* mConnectionStatusPane;

	bool mWasMaximized;
};

extern MainWindow* gMainWindow;

#endif // MAINWINDOW_H
