#include "editorstack.h"
#include "windowmanager.h"
#include <QDebug>

EditorStack::EditorStack(QWidget *parent, WindowManager* windowManager, EditorStack* copyParent) :
	QFrame(parent)
{

	mWindowManager = windowManager;
	mLayout = new QVBoxLayout(this);
	mLayout->setMargin(0);

	//	Don't show a border if this is the root view; that would look silly.
	if (parent == windowManager)
		setFrameStyle(QFrame::NoFrame);
	else
	{
		setFrameStyle(QFrame::Panel | QFrame::Plain);
		setLineWidth(2);
		setActive(false);
	}

	if (copyParent)
	{
		mStackedWidget = copyParent->mStackedWidget;
		mStackedWidget->setParent(this);
		mEditors.append(copyParent->mEditors);
	}
	else
	{
		//	By default, an EditorStack always starts out as a stack, not a split.
		mStackedWidget = new QStackedWidget(this);
	}

	mLayout->addWidget(mStackedWidget);
	mSplitWidget = NULL;

}

void EditorStack::displayEditor(Editor* editor)
{
	//	Nothing to do if already displayed.
	if (mStackedWidget->currentWidget() == editor) return;

	mStackedWidget->setCurrentWidget(editor);
	mWindowManager->notifyEditorChanged(this);
}

void EditorStack::displayFile(BaseFile* file)
{
	//	First, see if there is already an editor in this stack for the file...
	foreach (Editor* editor, mEditors)
	{
		if (editor->getLocation() == file->getLocation())
		{
			displayEditor(editor);
			return;
		}
	}

	//	If not, create a new editor for the file.
	createEditor(file);
}

void EditorStack::createEditor(BaseFile* file)
{
	Editor* newEditor = new Editor(file);
	mEditors.append(newEditor);
	mStackedWidget->addWidget(newEditor);

	displayEditor(newEditor);
}

void EditorStack::fileClosed(BaseFile* file)
{
	if (isSplit())
	{
		//	TODO: Pass through to children.
	}
	else
	{
		//	Deal with it locally.
		for (int i = 0; i < mEditors.length(); i++)
		{
			if (mEditors[i]->getFile() == file)
			{
				if (getCurrentEditor() == mEditors[i])
				{
					//	TODO: Move on to the next editor in order...
				}

				mEditors.removeAt(i);
				break;
			}
		}
	}
}

void EditorStack::setActive(bool active)
{
	if (parent() == mWindowManager) return;

	QPalette p;
	if (active)
		p.setColor(QPalette::WindowText, p.color(QPalette::Highlight));
	else
		p.setColor(QPalette::WindowText, p.color(QPalette::Window));

	setPalette(p);
}

void EditorStack::split(Qt::Orientation orientation)
{
	//	Remove the QStackedWidget and its contents...
	mLayout->removeWidget(mStackedWidget);
	mStackedWidget->setParent(NULL);

	//	Remove any frames; only split view children have frames.
	setFrameStyle(QFrame::NoFrame);
	setLineWidth(0);
	setMidLineWidth(0);

	//	Create a split widget
	mSplitWidget = new QSplitter(this);
	mSplitWidget->setOrientation(orientation);
	mLayout->addWidget(mSplitWidget);
	mSplitWidget->show();

	//	Left/Top editor stack
	EditorStack* stackA = new EditorStack(mSplitWidget, mWindowManager, this);
	mStacks.append(stackA);
	mSplitWidget->addWidget(stackA);

	//	Right/Bottom editor stack
	EditorStack* stackB = new EditorStack(mSplitWidget, mWindowManager);
	mStacks.append(stackB);
	mSplitWidget->addWidget(stackB);

	//	Put the split in the center
	QList<int> sizes;
	sizes.append(1);
	sizes.append(1);
	mSplitWidget->setSizes(sizes);

	//	Constructor for 1st child EditorStack has already taken ownership of the stacked widget at this point.
	mStackedWidget = NULL;

	//	Make sure the new stack has a file on display
	if (stackA->getCurrentEditor())
		stackB->displayFile(stackA->getCurrentEditor()->getFile());
}

EditorStack* EditorStack::findStack(Editor* editor)
{
	if (isSplit())
	{
		foreach (EditorStack* child, mStacks)
		{
			EditorStack* childResult = child->findStack(editor);
			if (childResult != NULL)
				return childResult;
		}
	}
	else
	{
		foreach (QObject* child, mStackedWidget->children())
			if (child == editor)
				return this;
	}

	return NULL;
}

void EditorStack::takeFocus()
{
	mWindowManager->setCurrentEditorStack(this);
}













