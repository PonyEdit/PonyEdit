#include "editor.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QTextCursor>
#include <QDebug>
#include "basefile.h"

Editor::Editor(const Location& location) : QStackedWidget()
{
	mEditor = new QTextEdit();
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

	showLoading();

	mFileLocation = location;
	mFile = mFileLocation.openFile();

	mEditor->setDocument(mFile->getTextDocument());
	mEditor->setAcceptRichText(false);
	mEditor->setFont(QFont("courier new", 11));

	connect(mFile, SIGNAL(openStatusChanged(int)), this, SLOT(openStatusChanged(int)));
}

void Editor::openStatusChanged(int openStatus)
{
	if (openStatus == BaseFile::Ready)
		setCurrentWidget(mEditor);
	else if (openStatus == BaseFile::Loading)
		showLoading();
	else if (openStatus == BaseFile::Error)
		showError(mFile->getError());
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

