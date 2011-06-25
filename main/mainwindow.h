#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QStatusBar>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolButton>
#include <QLabel>
#include <QTimer>

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
	void checkLicence(bool forceDialog = false);
	void showLicenceDialog();
	void updateTrialToolbar();

	void newFile();
	void openFile();
	void openSingleFile();
	void openSingleFile(const Location& loc);
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
	void closeHTMLPreview(bool visible);
	void options();

	void about();
	void checkForUpdates();

	void showErrorMessage(QString error);
	void showStatusMessage(QString message);

	void syntaxMenuOptionClicked();

	void currentEditorChanged();
	void updateSyntaxSelection();

	Editor* getCurrentEditor();

	void registerContextMenuItem(QDockWidget* widget) { mMenuControlledDockWidgets.append(widget); }
	void registerContextMenuItem(QToolBar* toolbar) { mMenuControlledToolBar.append(toolbar); }

protected:
	void closeEvent(QCloseEvent* event);
	void dragEnterEvent(QDragEnterEvent *);
	void dropEvent(QDropEvent *);

	QMenu *createPopupMenu();

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
	void addRecentFile(Location loc);

	void nextStartupPrompt();

	QTimer *mTrialTimer;

	FileList* mFileList;
	WindowManager* mWindowManager;
	QStatusBar* mStatusBar;
	QLabel* mStatusLine;

	QList<Location> mRecentFiles;
	QMenu* mRecentFilesMenu;

	QMap<QString, QAction*> mSyntaxMenuEntries;
	QAction* mCurrentSyntaxMenuItem;
	QMenu* mSyntaxMenu;

	QList<QDockWidget*> mMenuControlledDockWidgets;
	QList<QToolBar*> mMenuControlledToolBar;
	QToolBar* mTrialRemainingBar;
	QToolButton* mTrialRemainingButton;
	UnsavedChangesDialog* mUnsavedChangesDialog;
	ConnectionStatusPane* mConnectionStatusPane;

	bool mWasMaximized;
};

extern MainWindow* gMainWindow;

#endif // MAINWINDOW_H
