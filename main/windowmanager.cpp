#include <QDockWidget>

#include "windowmanager.h"
#include "file/openfilemanager.h"
#include "globaldispatcher.h"

WindowManager::WindowManager(QWidget *parent) :
    QWidget(parent)
{
	mParent = (MainWindow*)parent;

	mCurrentEditor = NULL;

	mLayout = new QVBoxLayout(this);
	mLayout->setSpacing(0);
	mLayout->setMargin(0);

	createSearchBar();
	createRegExpTester();

	connect(&gOpenFileManager, SIGNAL(fileClosed(BaseFile*)), this, SLOT(fileClosed(BaseFile*)), Qt::DirectConnection);
}

void WindowManager::displayFile(BaseFile *file)
{
	Location loc = file->getLocation();

	Location existingLoc;

	if(mCurrentEditor)
	{
		existingLoc = mCurrentEditor->getLocation();

		if(loc == existingLoc)
			return;
	}

	for(int ii = 0; ii < mEditors.length(); ii++)
	{
		existingLoc = mEditors[ii]->getLocation();
		if(loc == existingLoc)
		{
			mEditors[ii]->show();
			mEditors[ii]->setFocus();

			if(mCurrentEditor)
				mCurrentEditor->hide();

			mCurrentEditor = mEditors[ii];

			gDispatcher->emitSelectFile(file);
			emit currentChanged();

			return;
		}
	}

	Editor *newEditor = new Editor(file);
	mEditors.append(newEditor);

	mLayout->addWidget(newEditor);

	newEditor->show();
	newEditor->setFocus();

	if(mCurrentEditor)
		mCurrentEditor->hide();

	mCurrentEditor = newEditor;

	gDispatcher->emitSelectFile(file);
	emit currentChanged();
}

void WindowManager::fileClosed(BaseFile *file)
{
	for (int i = 0; i < mEditors.length(); i++)
	{
		Editor* e = mEditors[i];
		if (e->getFile() == file)
		{
			mEditors.removeAt(i);
			i--;

			if(e == mCurrentEditor)
				mCurrentEditor = NULL;

			delete e;
		}
	}

	if(!mCurrentEditor && mEditors.length() > 0)
		displayFile(mEditors[mEditors.length()-1]->getFile());
}

void WindowManager::nextWindow()
{
	Editor* next;

	if(mEditors.isEmpty())
		return;

	int currIdx = mEditors.indexOf(mCurrentEditor);

	if(currIdx + 1 == mEditors.length())
		next = mEditors[0];
	else
		next = mEditors[currIdx + 1];


	gDispatcher->emitSelectFile(next->getFile());
	displayFile(next->getFile());
}

void WindowManager::previousWindow()
{
	Editor *prev;

	if(mEditors.isEmpty())
		return;

	int currIdx = mEditors.indexOf(mCurrentEditor);


	if(currIdx == 0)
		prev = mEditors[mEditors.length() - 1];
	else
		prev = mEditors[currIdx - 1];

	gDispatcher->emitSelectFile(prev->getFile());
	displayFile(prev->getFile());
}

void WindowManager::find(const QString &text, bool backwards)
{
	int found;
	if (mCurrentEditor)
		found = find(mCurrentEditor, text, backwards, false, false);
}

void WindowManager::find(const QString &text, bool backwards, bool caseSensitive, bool useRegexp)
{
	int found;
	if (mCurrentEditor)
		found = find(mCurrentEditor, text, backwards, caseSensitive, useRegexp);
}

void WindowManager::globalFind(const QString &text, const QString &filePattern, bool backwards, bool caseSensitive, bool useRegexp)
{
	int found = 0;

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
	}
}

int WindowManager::find(Editor *editor, const QString &text, bool backwards, bool caseSensitive, bool useRegexp, bool loop)
{
	return editor->find(text, backwards, caseSensitive, useRegexp, loop);
}

void WindowManager::replace(const QString &findText, const QString &replaceText, bool all)
{
	int replaced;
	if (mCurrentEditor)
		replaced = replace(mCurrentEditor, findText, replaceText, false, false, all);
}

void WindowManager::replace(const QString &findText, const QString &replaceText, bool caseSensitive, bool useRegexp, bool all)
{
	int replaced;
	if (mCurrentEditor)
		replaced = replace(mCurrentEditor, findText, replaceText, caseSensitive, useRegexp, all);
}

void WindowManager::globalReplace(const QString &findText, const QString &replaceText, const QString &filePattern, bool caseSensitive, bool useRegexp, bool all)
{
	int replaced = 0;

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
	}
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

	mSearchBarWrapper->hide();
	mSearchBarWrapper->setTitleBarWidget(new QWidget(this));
	connect(mSearchBar, SIGNAL(closeRequested()), mSearchBarWrapper, SLOT(hide()));
	connect(mSearchBar, SIGNAL(find(QString,bool)), this, SLOT(find(QString,bool)));
	connect(mSearchBar, SIGNAL(replace(QString,QString,bool)), this, SLOT(replace(QString,QString,bool)));
	mSearchBarWrapper->setObjectName("Search Bar");
}

void WindowManager::showSearchBar()
{
	mSearchBarWrapper->show();
	mSearchBar->takeFocus();
}

void WindowManager::createRegExpTester()
{
	mRegExpTester = new RegExpTester();
	mRegExpTesterWrapper = new QDockWidget(tr("Regular Expression Tester"), 0);
	mRegExpTesterWrapper->setWidget(mRegExpTester);

	mParent->addDockWidget(Qt::BottomDockWidgetArea, mRegExpTesterWrapper, Qt::Horizontal);

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

