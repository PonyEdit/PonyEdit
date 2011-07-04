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

class MainWindow;
class EditorPanel;

class WindowManager : public QWidget
{
    Q_OBJECT
public:
    explicit WindowManager(QWidget *parent = 0);
	~WindowManager();

	void displayFile(BaseFile *file);

	Editor* currentEditor();
	QList<Editor*>* getEditors();

	void setCurrentEditorStack(EditorPanel* stack);
	void editorFocusSet(CodeEditor* newFocus);

signals:
	void currentChanged();
	void splitChanged();

public slots:
	void fileClosed(BaseFile* file);

	void findNext();
	void findPrevious();

	void find(const QString& text, bool backwards);
	void find(const QString& text, bool backwards, bool caseSensitive, bool useRegexp);
	void globalFind(const QString& text, const QString& filePattern, bool backwards, bool caseSensitive, bool useRegexp);

	void replace(const QString& findText, const QString& replaceText, bool all);
	void replace(const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegexp, bool all);
	void globalReplace(const QString& findText, const QString& replaceText, const QString& filePattern, bool caseSensitive, bool useRegexp, bool all);

	void showSearchBar();
	void hideSearchBar();
	void showRegExpTester();

	void previousWindow();
	void nextWindow();

	void notifyEditorChanged(EditorPanel* stack);	//	Called by EditorStacks, to notify the WindowManager when their current editor changes.

	void splitVertically();
	void splitHorizontally();
	bool isSplit();

private:
	int find(Editor* editor, const QString& text, bool backwards, bool caseSensitive, bool useRegexp, bool loop = true);
	int replace(Editor* editor, const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegexp, bool all);

	void createSearchBar();
	void createRegExpTester();

	MainWindow *mParent;

	QList<Editor*> mEditors;
	EditorPanel* mCurrentEditorStack;

	EditorPanel* mRootEditorStack;
	QLayout* mLayout;

	QDockWidget* mSearchBarWrapper;
	SearchBar* mSearchBar;

	QDockWidget* mRegExpTesterWrapper;
	RegExpTester* mRegExpTester;
};

//	One and only WindowManager object, created by MainWindow.
extern WindowManager* gWindowManager;

#endif // WINDOWMANAGER_H









