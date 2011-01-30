#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QWidget>
#include <QSplitter>
#include <QList>
#include <QMap>
#include <QVBoxLayout>

#include "editor/editor.h"
#include "file/basefile.h"

class WindowManager : public QWidget
{
    Q_OBJECT
public:
    explicit WindowManager(QWidget *parent = 0);

	/*void splitCurrent();
	void unsplitCurrent();
	void unsplitAll();

	QList<Editor*> currentEditors();
	QList<Editor*> currentSplitEditors();
	Editor* currentEditor();*/

	void displayFile(BaseFile *file);

	inline Editor* currentEditor() { return mCurrentEditor; }
	inline QList<Editor*>* getEditors() { return &mEditors; }

signals:
	void currentChanged();

public slots:
	void fileClosed(BaseFile* file);

private:
	QList<Editor*> mEditors;
	Editor *mCurrentEditor;
	QVBoxLayout *mLayout;

	/*QMap< QSplitter*, QList<Editor*> > mLeftEditors;
	QMap< QSplitter*, QList<Editor*> > mRightEditors;

	QList<QSplitter*> mSplitters;*/
};

#endif // WINDOWMANAGER_H
