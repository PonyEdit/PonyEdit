#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QWidget>
#include <QSplitter>
#include <QList>
#include <QMap>
#include <QVBoxLayout>

#include "editor/editor.h"
#include "file/basefile.h"
#include "searchbar.h"
#include "mainwindow.h"
#include "regexptester.h"
#include "searchresults.h"
#include "searchresultmodel.h"

class MainWindow;
class EditorPanel;

class WindowManager : public QWidget
{
    Q_OBJECT
public:
    explicit WindowManager(QWidget *parent = 0);
	~WindowManager();

	void displayFile(BaseFile *file);
	void displayLocation(const Location& location);

	BaseFile* getCurrentFile();
	Editor* currentEditor();

	void setCurrentEditorPanel(EditorPanel* stack);
	void editorFocusSet(CodeEditor* newFocus);

	inline EditorPanel* getCurrentPanel() const { return mCurrentEditorPanel; }
	inline void lockEditorSelection() { mEditorSelectionLocked = true; }
	inline void unlockEditorSelection() { mEditorSelectionLocked = false; }
	EditorPanel* getFirstPanel();
	EditorPanel* getLastPanel();

	void showSearchResults(const QList<SearchResultModel::Result>& results, bool showReplaceOptions);
	void showAndSelect(const Location& location, int lineNumber, int start, int length);
	void hideSearchResults();

signals:
	void currentChanged();
	void splitChanged();

public slots:
	void fileClosed(BaseFile* file);

	void findNext();
	void findPrevious();

	void findInCurrentEditor(const QString& text, bool backwards, bool caseSensitive = false, bool useRegExp = false);
	void replaceInCurrentEditor(const QString& text, const QString& replaceText, bool all);
	void searchInFiles(const QList<BaseFile*> files, const QString& text, bool caseSensitive, bool useRegExp, bool showReplaceOptions);

	void showSearchBar();
	void hideSearchBar();
	void showRegExpTester();

	void previousWindow();
	void nextWindow();

	void notifyEditorChanged(EditorPanel* stack);	//	Called by EditorStacks, to notify the WindowManager when their current editor changes.

	void splitVertically();
	void splitHorizontally();
	void removeSplit();
	void removeAllSplits();
	bool isSplit();

	void nextSplit();
	void previousSplit();

private:
	int find(Editor* editor, const QString& text, bool backwards, bool caseSensitive, bool useRegexp, bool loop = true);
	int replace(Editor* editor, const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegexp, bool all);

	void createSearchBar();
	void createRegExpTester();
	void createSearchResults();

	MainWindow *mParent;

	EditorPanel* mCurrentEditorPanel;
	bool mEditorSelectionLocked;	//	While rearranging editors, it is a good idea to lock editor seleciton.

	EditorPanel* mRootEditorPanel;
	QLayout* mLayout;

	QDockWidget* mSearchBarWrapper;
	SearchBar* mSearchBar;

	QDockWidget* mRegExpTesterWrapper;
	RegExpTester* mRegExpTester;

	QDockWidget* mSearchResultsWrapper;
	SearchResults* mSearchResults;
};

//	One and only WindowManager object, created by MainWindow.
extern WindowManager* gWindowManager;

#endif // WINDOWMANAGER_H









