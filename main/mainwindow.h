#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>
#include <QTextEdit>
#include <QStatusBar>
#include <QLabel>

#include "file/location.h"
#include "windowmanager.h"

class Editor;
class FileList;
class BaseFile;
class SearchBar;
class UnsavedChangesDialog;
class ConnectionStatusPane;
class WindowManager;

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
	void checkLicence();

	void newFile();
	void openFile();
	void openSingleFile(Location* loc = NULL);
	void saveFile();
	void saveFileAs();
	void saveAllFiles();
	void closeFile();
	void closeAllFiles();
	void closeAllExceptCurrentFile();
	void fileSelected(BaseFile* file);
	void reloadFile();

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

	void showAdvancedSearch();

	void resetZoom();
	void zoomIn();
	void zoomOut();

	void toggleFullScreen();

	void showHTMLPreview();
	void options();

	void about();
	void checkForUpdates();

	void showErrorMessage(QString error);
	void showStatusMessage(QString message);

	void syntaxMenuOptionClicked();

	void currentEditorChanged();
	void updateSyntaxSelection();

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

	void restoreState();

	void updateRecentFilesMenu();
	void addRecentFile(Location* loc);

	void nextStartupPrompt();

	FileList* mFileList;
	WindowManager* mWindowManager;
	QStatusBar* mStatusBar;
	QLabel* mStatusLine;

	QList<Location*> mRecentFiles;
	QMenu* mRecentFilesMenu;

	QMap<QString, QAction*> mSyntaxMenuEntries;
	QAction* mCurrentSyntaxMenuItem;
	QMenu* mSyntaxMenu;

	UnsavedChangesDialog* mUnsavedChangesDialog;

	ConnectionStatusPane* mConnectionStatusPane;

	bool mWasMaximized;
};

extern MainWindow* gMainWindow;

#endif // MAINWINDOW_H
