#include <QDockWidget>
#include <QtAlgorithms>
#include <QDebug>

#include "windowmanager.h"
#include "file/openfilemanager.h"
#include "globaldispatcher.h"
#include "editorstack.h"

bool editorSortLessThan(const Editor* e1, const Editor* e2)
{
	Location loc1 = e1->getLocation();
	Location loc2 = e2->getLocation();

	if(loc1.getParentPath() == loc2.getParentPath())
		return loc1.getPath() < loc2.getPath();

	return loc1.getParentPath() < loc2.getParentPath();
}

WindowManager::WindowManager(QWidget *parent) :
    QWidget(parent)
{
	mParent = (MainWindow*)parent;
	mCurrentEditorStack = NULL;

	//	Create a root editor stack
	mLayout = new QVBoxLayout(this);
	mLayout->setMargin(0);
	mRootEditorStack = new EditorStack(this, this);
	mLayout->addWidget(mRootEditorStack);
	setCurrentEditorStack(mRootEditorStack);

	createSearchBar();
	createRegExpTester();

	connect(&gOpenFileManager, SIGNAL(fileClosed(BaseFile*)), this, SLOT(fileClosed(BaseFile*)), Qt::DirectConnection);
}

WindowManager::~WindowManager()
{
	mSearchBar->deleteLater();
	mSearchBarWrapper->deleteLater();
	mRegExpTester->deleteLater();
	mRegExpTesterWrapper->deleteLater();
}

void WindowManager::displayFile(BaseFile *file)
{
	Location loc = file->getLocation();
	mCurrentEditorStack->displayFile(file);
}

void WindowManager::fileClosed(BaseFile *file)
{
	//	Tell all editor stacks that the file's gone.
	mRootEditorStack->fileClosed(file);
}

void WindowManager::setCurrentEditorStack(EditorStack* stack)
{
	if (mCurrentEditorStack == stack) return;

	if (mCurrentEditorStack != NULL)
		mCurrentEditorStack->setActive(false);

	mCurrentEditorStack = stack;
	mCurrentEditorStack->setActive(true);
}

void WindowManager::nextWindow()
{
	/*Editor* next;

	if(mEditors.isEmpty())
		return;

	int currIdx = mEditors.indexOf(mCurrentEditor);

	if(currIdx + 1 == mEditors.length())
		next = mEditors[0];
	else
		next = mEditors[currIdx + 1];


	gDispatcher->emitSelectFile(next->getFile());
	displayFile(next->getFile());*/
}

void WindowManager::previousWindow()
{
	/*Editor *prev;

	if(mEditors.isEmpty())
		return;

	int currIdx = mEditors.indexOf(mCurrentEditor);


	if(currIdx == 0)
		prev = mEditors[mEditors.length() - 1];
	else
		prev = mEditors[currIdx - 1];

	gDispatcher->emitSelectFile(prev->getFile());
	displayFile(prev->getFile());*/
}

void WindowManager::find(const QString &text, bool backwards)
{
	/*int found;
	if (mCurrentEditor)
		found = find(mCurrentEditor, text, backwards, false, false);*/
}

void WindowManager::find(const QString &text, bool backwards, bool caseSensitive, bool useRegexp)
{
	/*int found;
	if (mCurrentEditor)
		found = find(mCurrentEditor, text, backwards, caseSensitive, useRegexp);*/
}

void WindowManager::globalFind(const QString &text, const QString &filePattern, bool backwards, bool caseSensitive, bool useRegexp)
{
	/*int found = 0;

	Editor* current;
	BaseFile* file;
	Location loc;

#ifdef Q_OS_WIN
	QRegExp regexp(filePattern, Qt::CaseInsensitive, QRegExp::Wildcard);
#else
	QRegExp regexp(filePattern, Qt::CaseInsensitive, QRegExp::WildcardUnix);
#endif

	int filesSearched = 0;

	for(int ii = mEditors.indexOf(mCurrentEditor); filesSearched < mEditors.length(); (backwards)?(ii--):(ii++))
	{
		current = mEditors[ii];
		if(current)
		{
			file = current->getFile();
			loc = file->getLocation();

			if(regexp.exactMatch(loc.getDisplayPath()) || regexp.exactMatch(loc.getLabel()))
			{
				gDispatcher->emitSelectFile(file);
				current->setFocus();
				if(filesSearched > 0)
				{
					if(!backwards)
						current->gotoLine(1);
					else
						current->gotoEnd();
				}

				found += find(current, text, backwards, caseSensitive, useRegexp, false);
				if(found)
					break;
			}
		}
		filesSearched++;

		if(ii == 0 && backwards)
			ii = mEditors.length();
		else if(ii == mEditors.length() - 1 && !backwards)
			ii = -1;
	}*/
}

int WindowManager::find(Editor *editor, const QString &text, bool backwards, bool caseSensitive, bool useRegexp, bool loop)
{
	return editor->find(text, backwards, caseSensitive, useRegexp, loop);
}

void WindowManager::replace(const QString &findText, const QString &replaceText, bool all)
{
	/*int replaced;
	if (mCurrentEditor)
		replaced = replace(mCurrentEditor, findText, replaceText, false, false, all);*/
}

void WindowManager::replace(const QString &findText, const QString &replaceText, bool caseSensitive, bool useRegexp, bool all)
{
	/*int replaced;
	if (mCurrentEditor)
		replaced = replace(mCurrentEditor, findText, replaceText, caseSensitive, useRegexp, all);*/
}

void WindowManager::globalReplace(const QString &findText, const QString &replaceText, const QString &filePattern, bool caseSensitive, bool useRegexp, bool all)
{
	/*int replaced = 0;

	Editor* current;
	BaseFile* file;
	Location loc;

#ifdef Q_OS_WIN
	QRegExp regexp(filePattern, Qt::CaseInsensitive, QRegExp::Wildcard);
#else
	QRegExp regexp(filePattern, Qt::CaseInsensitive, QRegExp::WildcardUnix);
#endif

	int ii;
	if(all)
		ii = 0;
	else
		ii = mEditors.indexOf(mCurrentEditor);

	for(; ii < mEditors.length(); ii++)
	{
		current = mEditors[ii];
		if(current)
		{
			file = current->getFile();
			loc = file->getLocation();

			if(regexp.exactMatch(loc.getDisplayPath()) || regexp.exactMatch(loc.getLabel()))
			{
				if(!all)
				{
					gDispatcher->emitSelectFile(file);
					current->setFocus();
				}

				replaced += replace(current, findText, replaceText, caseSensitive, useRegexp, all);
				if(replaced && !all)
					break;
			}
		}
	}*/
}

int WindowManager::replace(Editor *editor, const QString &findText, const QString &replaceText, bool caseSensitive, bool useRegexp, bool all)
{
	return editor->replace(findText, replaceText, caseSensitive, useRegexp, all);
}

void WindowManager::findNext()
{
	mSearchBar->findNext();
}

void WindowManager::findPrevious()
{
	mSearchBar->findPrev();
}

void WindowManager::createSearchBar()
{
	mSearchBar = new SearchBar();
	mSearchBarWrapper = new QDockWidget("Search", 0, Qt::FramelessWindowHint);
	mSearchBarWrapper->setFeatures(QDockWidget::DockWidgetClosable);
	mSearchBarWrapper->setWidget(mSearchBar);

	mParent->addDockWidget(Qt::BottomDockWidgetArea, mSearchBarWrapper, Qt::Horizontal);
	mParent->registerContextMenuItem(mSearchBarWrapper);

	mSearchBarWrapper->hide();
	mSearchBarWrapper->setTitleBarWidget(new QWidget(this));
	connect(mSearchBar, SIGNAL(closeRequested()), this, SLOT(hideSearchBar()));
	connect(mSearchBar, SIGNAL(find(QString,bool)), this, SLOT(find(QString,bool)));
	connect(mSearchBar, SIGNAL(replace(QString,QString,bool)), this, SLOT(replace(QString,QString,bool)));
	mSearchBarWrapper->setObjectName("Search Bar");
}

void WindowManager::showSearchBar()
{
	mSearchBarWrapper->show();
	mSearchBar->takeFocus();
}

void WindowManager::hideSearchBar()
{
	mSearchBarWrapper->hide();
	Editor* editor = currentEditor();
	if (editor) editor->getCodeEditor()->setFocus();
}

void WindowManager::createRegExpTester()
{
	mRegExpTester = new RegExpTester();
	mRegExpTesterWrapper = new QDockWidget(tr("Regular Expression Tester"), 0);
	mRegExpTesterWrapper->setWidget(mRegExpTester);

	mParent->addDockWidget(Qt::BottomDockWidgetArea, mRegExpTesterWrapper, Qt::Horizontal);
	mParent->registerContextMenuItem(mRegExpTesterWrapper);

	mRegExpTesterWrapper->hide();
	mRegExpTesterWrapper->setObjectName("Search Bar");
}

void WindowManager::showRegExpTester()
{
	mRegExpTesterWrapper->show();

	Editor* editor = currentEditor();
	QString selectedText;
	if (editor)
	{
		CodeEditor* codeEditor = editor->getCodeEditor();
		selectedText = codeEditor->textCursor().selectedText();
	}

	mRegExpTester->takeFocus(selectedText);
}

void WindowManager::notifyEditorChanged(EditorStack* stack)
{
	if (stack == mCurrentEditorStack)
		emit currentChanged();
}

void WindowManager::splitVertically()
{
	EditorStack* stack = mCurrentEditorStack;
	stack->split(Qt::Horizontal);
	setCurrentEditorStack(stack->getFirstChild());
	emit splitChanged();
}

void WindowManager::splitHorizontally()
{
	EditorStack* stack = mCurrentEditorStack;
	mCurrentEditorStack->split(Qt::Vertical);
	setCurrentEditorStack(stack->getFirstChild());
	emit splitChanged();
}














