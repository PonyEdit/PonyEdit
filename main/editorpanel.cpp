#include "editorpanel.h"
#include "windowmanager.h"
#include <QDebug>

EditorPanel::EditorPanel(QWidget* parent, EditorStack* inheritedStack) :
	QFrame(parent)
{
	mLayout = new QVBoxLayout(this);
	mLayout->setMargin(0);

	//	Don't show a border if this is the root view
	if (isRootPanel())
		setFrameStyle(QFrame::NoFrame);
	else
	{
		setFrameStyle(QFrame::Panel | QFrame::Plain);
		setLineWidth(2);
		setActive(false);
	}

	if (inheritedStack)
	{
		mEditorStack = inheritedStack;
		mEditorStack->setParent(this);
		mEditorStack->show();
	}
	else
		mEditorStack = new EditorStack(this);

	mLayout->addWidget(mEditorStack);
	mSplitWidget = NULL;

}

void EditorPanel::displayEditor(Editor* editor)
{
	if (isSplit()) return;
	mEditorStack->displayEditor(editor);
}

void EditorPanel::displayFile(BaseFile* file)
{
	if (isSplit()) return;
	mEditorStack->displayFile(file);
}

void EditorPanel::fileClosed(BaseFile* file)
{
	if (isSplit())
	{
		foreach (EditorPanel* child, mChildPanels)
			child->fileClosed(file);
	}
	else
		mEditorStack->fileClosed(file);
}

void EditorPanel::setActive(bool active)
{
	if (parent() == gWindowManager) return;

	QPalette p;
	if (active)
		p.setColor(QPalette::WindowText, p.color(QPalette::Highlight));
	else
		p.setColor(QPalette::WindowText, p.color(QPalette::Window));

	setPalette(p);
}

void EditorPanel::split(Qt::Orientation orientation)
{
	//	Remove the current EditorStack...
	mLayout->removeWidget(mEditorStack);
	mEditorStack->setParent(NULL);

	//	Remove the frame (if any); split panels do not have frames; their children might
	setFrameStyle(QFrame::NoFrame);

	//	Create a splitter
	mSplitWidget = new QSplitter(this);
	mSplitWidget->setOrientation(orientation);
	mLayout->addWidget(mSplitWidget);
	mSplitWidget->show();

	//	Left/Top editor stack
	EditorPanel* stackA = new EditorPanel(mSplitWidget, mEditorStack);
	mChildPanels.append(stackA);
	mSplitWidget->addWidget(stackA);

	//	Right/Bottom editor stack
	EditorPanel* stackB = new EditorPanel(mSplitWidget);
	mChildPanels.append(stackB);
	mSplitWidget->addWidget(stackB);

	//	Put the split in the center
	QList<int> sizes;
	sizes.append(1);
	sizes.append(1);
	mSplitWidget->setSizes(sizes);

	//	EditorStack is now owned by the child Panel; not this panel.
	mEditorStack = NULL;

	//	The new stack should point at the same file as the first stack.
	if (stackA->getCurrentEditor())
		stackB->displayFile(stackA->getCurrentEditor()->getFile());
}

EditorPanel* EditorPanel::findStack(Editor* editor)
{
	if (isSplit())
	{
		foreach (EditorPanel* child, mChildPanels)
		{
			EditorPanel* childResult = child->findStack(editor);
			if (childResult != NULL)
				return childResult;
		}
	}
	else
	{
		foreach (QObject* child, mEditorStack->children())
			if (child == editor)
				return this;
	}

	return NULL;
}

void EditorPanel::takeFocus()
{
	gWindowManager->setCurrentEditorStack(this);
}

Editor* EditorPanel::getCurrentEditor() const
{
	 return mEditorStack->getCurrentEditor();
}












