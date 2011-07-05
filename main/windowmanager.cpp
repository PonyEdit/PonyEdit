#include <QDockWidget>
#include <QtAlgorithms>
#include <QDebug>

#include "windowmanager.h"
#include "file/openfilemanager.h"
#include "globaldispatcher.h"
#include "editorpanel.h"

WindowManager* gWindowManager = NULL;

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
	mEditorSelectionLocked = false;
	mParent = (MainWindow*)parent;
	mCurrentEditorPanel = NULL;
	gWindowManager = this;

	//	Create a root editor stack
	mLayout = new QVBoxLayout(this);
	mLayout->setMargin(0);
	mRootEditorPanel = new EditorPanel(this);
	mLayout->addWidget(mRootEditorPanel);
	setCurrentEditorStack(mRootEditorPanel);

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
	mCurrentEditorPanel->displayFile(file);
}

void WindowManager::fileClosed(BaseFile *file)
{
	//	Tell all editor stacks that the file's gone.
	mRootEditorPanel->fileClosed(file);
}

void WindowManager::setCurrentEditorStack(EditorPanel* stack)
{
	if (mEditorSelectionLocked) return;

	if (mCurrentEditorPanel == stack) return;

	if (mCurrentEditorPanel != NULL)
		mCurrentEditorPanel->setActive(false);

	mCurrentEditorPanel = stack;

	if (mCurrentEditorPanel != NULL)
		mCurrentEditorPanel->setActive(true);
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

void WindowManager::notifyEditorChanged(EditorPanel* stack)
{
	if (stack == mCurrentEditorPanel)
		emit currentChanged();
}

void WindowManager::splitVertically()
{
	EditorPanel* stack = mCurrentEditorPanel;
	stack->split(Qt::Horizontal);
	setCurrentEditorStack(stack->getFirstChild());
	emit splitChanged();
}

void WindowManager::splitHorizontally()
{
	EditorPanel* stack = mCurrentEditorPanel;
	mCurrentEditorPanel->split(Qt::Vertical);
	setCurrentEditorStack(stack->getFirstChild());
	emit splitChanged();
}

Editor* WindowManager::currentEditor()
{
	return mCurrentEditorPanel->getCurrentEditor();
}

QList<Editor*>* WindowManager::getEditors()
{
	return &mEditors;
}

bool WindowManager::isSplit()
{
	return mRootEditorPanel->isSplit();
}

void WindowManager::removeSplit()
{
	mCurrentEditorPanel->unsplit();
	emit splitChanged();
}

void WindowManager::removeAllSplits()
{
	mRootEditorPanel->unsplit();
	emit splitChanged();
}

void WindowManager::nextSplit()
{
	if (mCurrentEditorPanel == NULL) return;
	EditorPanel* nextPanel = mCurrentEditorPanel->findNextPanel();
	if (nextPanel)
		setCurrentEditorStack(nextPanel);
}

void WindowManager::previousSplit()
{
	if (mCurrentEditorPanel == NULL) return;
	EditorPanel* nextPanel = mCurrentEditorPanel->findPreviousPanel();
	if (nextPanel)
		setCurrentEditorStack(nextPanel);
}

EditorPanel* WindowManager::getFirstPanel()
{
	EditorPanel* panel = mRootEditorPanel;
	while (panel->isSplit())
		panel = panel->getFirstChild();
	return panel;
}

EditorPanel* WindowManager::getLastPanel()
{
	EditorPanel* panel = mRootEditorPanel;
	while (panel->isSplit())
		panel = panel->getSecondChild();
	return panel;
}






