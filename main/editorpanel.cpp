#include "editorpanel.h"
#include "windowmanager.h"
#include <QDebug>

EditorPanel::EditorPanel(QWidget* parent, EditorPanel* parentPanel, EditorStack* inheritedStack) :
	QFrame(parent)
{
	mParentPanel = parentPanel;
	mLayout = new QVBoxLayout(this);
	mLayout->setMargin(0);
	mSplitWidget = NULL;

	setupBorder();

	if (inheritedStack)
	{
		mEditorStack = inheritedStack;
		mEditorStack->setParent(this);
		mEditorStack->show();
	}
	else
		mEditorStack = new EditorStack(this);

	mLayout->addWidget(mEditorStack);

}

void EditorPanel::setupBorder()
{
	//	Don't show a border if this is the root view
	if (isSplit() || isRootPanel())
		setFrameStyle(QFrame::NoFrame);
	else
	{
		setFrameStyle(QFrame::Panel | QFrame::Plain);
		setLineWidth(2);
		setActive(false);
	}
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

void EditorPanel::unsplit()
{
	//	If this is not a split panel, pass the request up to my parent.
	if (!isSplit())
	{
		if (mParentPanel) mParentPanel->unsplit();
		return;
	}

	EditorPanel* currentEditor = gWindowManager->getCurrentPanel();
	gWindowManager->setCurrentEditorStack(this);
	gWindowManager->lockEditorSelection();

	//	Pick which descendant to keep during the unsplit. Attempt 1: See if
	//	the current panel is a child of this split panel.
	EditorPanel* keeper = NULL;
	EditorPanel* scanCurrent = currentEditor;
	while (scanCurrent != NULL && scanCurrent != this)
		scanCurrent = scanCurrent->getParentPanel();
	if (scanCurrent != NULL)
		keeper = currentEditor;

	//	Attempt 2: If the current panel was not found, just use the first one
	if (keeper == NULL)
	{
		keeper = this;
		while (keeper->isSplit())
			keeper = keeper->getFirstChild();
	}

	//	Remove the keeper's editor stack
	EditorStack* keepStack = keeper->mEditorStack;
	keeper->layout()->removeWidget(keepStack);
	keepStack->setParent(NULL);

	//	Nuke all children of me
	mChildPanels.clear();
	delete mSplitWidget;
	mSplitWidget = NULL;

	mEditorStack = keepStack;
	mEditorStack->setParent(this);
	mLayout->addWidget(mEditorStack);

	setupBorder();

	gWindowManager->unlockEditorSelection();
}

void EditorPanel::split(Qt::Orientation orientation)
{
	//	Remove the current EditorStack...
	mLayout->removeWidget(mEditorStack);
	mEditorStack->setParent(NULL);

	//	Create a splitter
	mSplitWidget = new QSplitter(this);
	mSplitWidget->setOrientation(orientation);
	mLayout->addWidget(mSplitWidget);
	mSplitWidget->show();

	setupBorder();

	//	Left/Top editor stack
	EditorPanel* stackA = new EditorPanel(mSplitWidget, this, mEditorStack);
	mChildPanels.append(stackA);
	mSplitWidget->addWidget(stackA);

	//	Right/Bottom editor stack
	EditorPanel* stackB = new EditorPanel(mSplitWidget, this);
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












