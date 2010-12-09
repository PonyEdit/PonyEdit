#include "editor.h"
#include <QGridLayout>
#include <QSpacerItem>
#include <QTextCursor>
#include <QDebug>
#include "basefile.h"
#include "syntaxhighlighter.h"

Editor::Editor(BaseFile* file) : QStackedWidget()
{
	mEditor = new CodeEditor();
	mFirstOpen = true;
	addWidget(mEditor);

	mWorkingPane = new QWidget();
	QGridLayout* layout = new QGridLayout(mWorkingPane);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 0, 0, 1, 4);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 1, 0);
	mWorkingIcon = new QLabel();
	mWorkingIcon->setFixedSize(16, 16);
	layout->addWidget(mWorkingIcon, 1, 1, 1, 1);
	mWorkingText = new QLabel();
	layout->addWidget(mWorkingText, 1, 2, 1, 1);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding), 1, 3);

	mProgressBar = new QProgressBar();
	mProgressBar->setMaximum(100);
	mProgressBar->setValue(0);
	layout->addWidget(mProgressBar, 2, 1, 1, 2);
	layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), 3, 0, 1, 4);

	addWidget(mWorkingPane);

	mFile = file;
	mFile->editorAttached(this);
	connect(mFile, SIGNAL(openStatusChanged(int)), this, SLOT(openStatusChanged(int)));
	connect(mFile, SIGNAL(fileOpenProgress(int)), this, SLOT(fileOpenProgress(int)));
	openStatusChanged(mFile->getOpenStatus());

	mEditor->setDocument(mFile->getTextDocument());
	new SyntaxHighlighter(mEditor->document());
	mEditor->setFont(QFont("inconsolata", 11));
}

Editor::~Editor()
{
	mFile->editorDetached(this);
}

void Editor::fileOpenProgress(int percent)
{
	mProgressBar->setValue(percent);
}

void Editor::openStatusChanged(int openStatus)
{
	switch (openStatus)
	{
		case BaseFile::Closed:
			break;

		case BaseFile::Loading:
			showLoading();
			break;

		case BaseFile::LoadError:
			showError(mFile->getError());
			break;

		case BaseFile::Ready:
			if (mFirstOpen)
			{
				mFirstOpen = false;
				mEditor->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
			}

		case BaseFile::Disconnected:
		case BaseFile::Reconnecting:
		case BaseFile::Repairing:
			setCurrentWidget(mEditor);
			if (hasFocus()) mEditor->setFocus();
			break;

		case BaseFile::Closing:
			showError("Closing file...");
			break;
	}
}

void Editor::showLoading()
{
	mWorkingIcon->setPixmap(QPixmap(":/icons/loading.png"));
	mWorkingText->setText("Loading ...");
	mProgressBar->show();
	setCurrentWidget(mWorkingPane);
}

void Editor::showError(const QString& error)
{
	mWorkingIcon->setPixmap(QPixmap(":/icons/error.png"));
	mWorkingText->setText(QString("Error: ") + error);
	mProgressBar->hide();
	setCurrentWidget(mWorkingPane);
}

void Editor::save()
{
	mFile->save();
}

void Editor::find(const QString& text, bool backwards)
{
	mEditor->find(text, (QTextDocument::FindFlags)(backwards ? QTextDocument::FindBackward : 0));
}

void Editor::fileClosed()
{
	delete this;
}



















