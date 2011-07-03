#ifndef EDITORSTACK_H
#define EDITORSTACK_H

#include <QWidget>
#include <QStackedWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <editor/editor.h>

class WindowManager;
class EditorStack : public QFrame
{
    Q_OBJECT
public:
	explicit EditorStack(QWidget* parent, WindowManager* windowManager, EditorStack* copyParent = NULL);

	void fileClosed(BaseFile* file);
	inline bool isSplit() const { return mSplitWidget != NULL; }
	EditorStack* findStack(Editor* editor);

	//	Public methods for split stacks
	inline EditorStack* getFirstChild() const { return mStacks[0]; }
	inline EditorStack* getSecondChild() const { return mStacks[1]; }

	//	Public methods for unsplit stacks
	void displayFile(BaseFile* file);
	void displayEditor(Editor* editor);
	inline Editor* getCurrentEditor() const { return (Editor*)mStackedWidget->currentWidget(); }
	void split(Qt::Orientation orientation);
	void setActive(bool active);
	void takeFocus();

protected:
	//	Protected methods for unsplit stacks
	void createEditor(BaseFile* file);

private:
	WindowManager* mWindowManager;
	QVBoxLayout* mLayout;

	//	Members for split stacks
	QSplitter* mSplitWidget;
	QList<EditorStack*> mStacks;

	//	Members for unsplit stacks
	QStackedWidget* mStackedWidget;
	QList<Editor*> mEditors;
};

#endif // EDITORSTACK_H
