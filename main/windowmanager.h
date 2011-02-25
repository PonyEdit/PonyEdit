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

class WindowManager : public QWidget
{
    Q_OBJECT
public:
    explicit WindowManager(QWidget *parent = 0);

	/*void splitCurrent();
	void unsplitCurrent();
	void unsplitAll();

	QList<Editor*> currentEditors();
	QList<Editor*> currentSplitEditors();*/

	void displayFile(BaseFile *file);

	inline Editor* currentEditor() { return mCurrentEditor; }
	inline QList<Editor*>* getEditors() { return &mEditors; }

signals:
	void currentChanged();

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
	void showRegExpTester();

	void previousWindow();
	void nextWindow();

private:
	int find(Editor* editor, const QString& text, bool backwards, bool caseSensitive, bool useRegexp, bool loop = true);
	int replace(Editor* editor, const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegexp, bool all);

	void createSearchBar();
	void createRegExpTester();

	MainWindow *mParent;

	QList<Editor*> mEditors;
	Editor *mCurrentEditor;
	QVBoxLayout *mLayout;

	QDockWidget* mSearchBarWrapper;
	SearchBar* mSearchBar;

	QDockWidget* mRegExpTesterWrapper;
	RegExpTester* mRegExpTester;

	/*QMap< QSplitter*, QList<Editor*> > mLeftEditors;
	QMap< QSplitter*, QList<Editor*> > mRightEditors;

	QList<QSplitter*> mSplitters;*/
};

#endif // WINDOWMANAGER_H
