#ifndef EDITORSTACK_H
#define EDITORSTACK_H

#include <QList>
#include <QStackedWidget>

class Editor;
class EditorPanel;
class BaseFile;

class EditorStack : public QStackedWidget
{
Q_OBJECT
public:
explicit EditorStack( EditorPanel *parent = 0 );

Editor* getCurrentEditor() const;
void displayEditor( Editor* editor );
void displayFile( BaseFile* file );
void fileClosed( BaseFile* file );

protected:
void createEditor( BaseFile* file );

private:
EditorPanel* mParentPanel;
QList< Editor* > mEditors;
};

#endif	// EDITORSTACK_H
