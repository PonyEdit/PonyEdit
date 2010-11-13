#include "editor.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QTextCursor>
#include <QDebug>
#include "basefile.h"

Editor::Editor(BaseFile* file) : QStackedWidget()
{
	mEditor = new CodeEditor();
	addWidget(mEditor);

	mWorkingPane = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout(mWorkingPane);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
	mWorkingIcon = new QLabel();
	mWorkingIcon->setFixedSize(16, 16);
	layout->addWidget(mWorkingIcon);
	mWorkingText = new QLabel();
	layout->addWidget(mWorkingText);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
	addWidget(mWorkingPane);

	mFile = file;
	mFile->editorAttached(this);
	connect(mFile, SIGNAL(openStatusChanged(int)), this, SLOT(openStatusChanged(int)));
	openStatusChanged(mFile->getOpenStatus());

	mEditor->setDocument(mFile->getTextDocument());
	mEditor->setFont(QFont("courier new", 11));
}

Editor::~Editor()
{
	mFile->editorDetached(this);
}

void Editor::openStatusChanged(int openStatus)
{
	switch (openStatus)
	{
		case BaseFile::Closed:
		case BaseFile::Loading:
			showLoading();
			break;

		case BaseFile::LoadError:
			showError(mFile->getError());
			break;

		case BaseFile::Ready:
		case BaseFile::Disconnected:
		case BaseFile::Reconnecting:
		case BaseFile::Repairing:
			setCurrentWidget(mEditor);
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
	setCurrentWidget(mWorkingPane);
}

void Editor::showError(const QString& error)
{
	mWorkingIcon->setPixmap(QPixmap(":/icons/error.png"));
	mWorkingText->setText(QString("Error: ") + error);
	setCurrentWidget(mWorkingPane);
}

void Editor::save()
{
	mFile->save();
}

