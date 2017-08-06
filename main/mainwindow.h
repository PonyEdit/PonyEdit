#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QtWidgets/QMainWindow>

#include "file/location.h"
#include "windowmanager.h"

class Editor;
class FileList;
class TabbedFileList;
class BaseFile;
class SearchBar;
class UnsavedChangesDialog;
class WindowManager;

#ifdef Q_OS_MAC
extern void qt_mac_set_dock_menu( QMenu *menu );
#endif

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow( QWidget *parent = 0 );
	~MainWindow();

public slots:
	void newFile();
	void openFile();
	void openSingleFile();
	void openSingleFile( const Location &loc );
	void saveFile();
	void saveFileAs();
	void saveAllFiles();
	void closeFile();
	void closeAllFiles();
	void closeAllExceptCurrentFile();
	void fileSelected( BaseFile *file );

	void print();

	void updateTitle();
	void updateTitle( BaseFile *file );

	void undo();
	void redo();
	void cut();
	void copy();
	void paste();
	void selectAll();

	void deleteLine();

	void showGotoLine();

	void showAdvancedSearch();

	void resetZoom();
	void zoomIn();
	void zoomOut();

	void toggleFullScreen();

	void showHTMLPreview();
	void closeHTMLPreview( bool visible );
	void options();

	void contextHelp();
	void about();
	void checkForUpdates();

	void showErrorMessage( QString error );
	void showStatusMessage( QString message );

	void syntaxMenuOptionClicked();

	void switchToQuickList();
	void switchtoTabbedList();

	void currentEditorChanged();
	void updateSyntaxSelection();

	Editor *getCurrentEditor();

	void openFileListChanged();
	void viewSplittingChanged();

	void registerContextMenuItem( QDockWidget *widget ) {
		mMenuControlledDockWidgets.append( widget );
	}
	void registerContextMenuItem( QToolBar *toolbar ) {
		mMenuControlledToolBar.append( toolbar );
	}

protected:
	void closeEvent( QCloseEvent *event );
	void dragEnterEvent( QDragEnterEvent * );
	void dropEvent( QDropEvent * );

	QMenu *createPopupMenu();

private:
	void createToolbar();
	void createFileMenu();
	void createViewMenu();
	void createEditMenu();
	void createToolsMenu();
	void createWindowMenu();
	void createHelpMenu();

	void createShortcuts();

	void createMacDockMenu();

	void restoreState();

	void updateRecentFilesMenu();
	void addRecentFile( Location loc );

	void nextStartupPrompt();

	FileList *      mFileList;
	TabbedFileList *mTabbedFileList;
	QStatusBar *    mStatusBar;
	QLabel *        mStatusLine;

	QList< Location > mRecentFiles;
	QMenu *           mRecentFilesMenu;

	QAction *mQuickListMenuItem;
	QAction *mTabbedListMenuItem;

	QList< QAction * > mActionsRequiringFiles;
	QList< QAction * > mActionsRequiringSplitViews;

	QMap< QString, QAction * > mSyntaxMenuEntries;
	QAction *                  mCurrentSyntaxMenuItem;
	QMenu *                    mSyntaxMenu;

	QList< QDockWidget * > mMenuControlledDockWidgets;
	QList< QToolBar * >    mMenuControlledToolBar;
	UnsavedChangesDialog * mUnsavedChangesDialog;

	bool mWasMaximized;
};

extern MainWindow *gMainWindow;

#endif // MAINWINDOW_H
