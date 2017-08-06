#ifndef EDITORPANEL_H
#define EDITORPANEL_H

HIDE_COMPILE_WARNINGS

#include <QWidget>
#include <QSplitter>
#include <QVBoxLayout>
#include <QDebug>

UNHIDE_COMPILE_WARNINGS

#include <editor/editor.h>
#include "editorstack.h"

class WindowManager;
extern WindowManager* gWindowManager;

class EditorPanel : public QFrame
{
    Q_OBJECT
public:
	explicit EditorPanel(QWidget* parent, EditorPanel* parentPanel = NULL, EditorStack* inheritedStack = NULL);
	~EditorPanel();

	EditorPanel(EditorPanel const&) = delete;
	EditorPanel& operator=(EditorPanel const&) = delete;
			
	void fileClosed(BaseFile* file);
	inline bool isSplit() const { return mSplitWidget != NULL; }
	inline bool isRootPanel() const { return parent() == (QObject*)gWindowManager; }
	inline EditorPanel* getParentPanel() const { return mParentPanel; }
	EditorPanel* findStack(Editor* editor);

	//	Public methods for split panels
	inline EditorPanel* getFirstChild() const { return mChildPanels[0]; }
	inline EditorPanel* getSecondChild() const { return mChildPanels[1]; }
	void unsplit();

	//	Public methods for unsplit panels
	void displayFile(BaseFile* file);
	void displayEditor(Editor* editor);
	Editor* getCurrentEditor() const;
	void split(Qt::Orientation orientation);
	void setActive(bool active);
	void takeFocus();

	EditorPanel* findNextPanel();
	EditorPanel* findPreviousPanel();

protected:
	void createEditor(BaseFile* file);
	void setupBorder();

private:
	QVBoxLayout* mLayout;
	EditorPanel* mParentPanel;

	//	Members for split panels
	QSplitter* mSplitWidget;
	QList<EditorPanel*> mChildPanels;

	//	Members for unsplit panels
	EditorStack* mEditorStack;
};

#endif // EDITORPANEL_H
