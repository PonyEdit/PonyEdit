#include "windowmanager.h"
#include "file/openfilemanager.h"

WindowManager::WindowManager(QWidget *parent) :
    QWidget(parent)
{
	mCurrentEditor = NULL;

	mLayout = new QVBoxLayout(this);
	mLayout->setSpacing(0);
	mLayout->setMargin(0);

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
