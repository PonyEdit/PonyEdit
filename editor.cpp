#include "editor.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QDebug>

Editor::Editor(const Location& location) : QStackedWidget()
{
	mDocument = NULL;

	mDocument = new QTextDocument();

	mEditor = new QTextEdit();
	mEditor->setDocument(mDocument);
	mEditor->setAcceptRichText(false);
	mEditor->setFont(QFont("courier new", 11));
	addWidget(mEditor);

	mWorkingPane = new QWidget();
	QHBoxLayout* layout = new QHBoxLayout(mWorkingPane);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
	mWorkingIcon = new QLabel();
	mWorkingIcon->setFixedSize(16, 16);
	mWorkingIcon->setPixmap(QPixmap(":/icons/loading.png"));
	layout->addWidget(mWorkingIcon);
	mWorkingText = new QLabel();
	mWorkingText->setText("Loading ...");
	layout->addWidget(mWorkingText);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
	addWidget(mWorkingPane);

	setCurrentWidget(mWorkingPane);

	mFileLocation = location;
	mFileLocation.asyncOpenFile(this, SLOT(openFileSuccessful(File*)), SLOT(openFileFailed(QString)));
}

void Editor::openFileFailed(const QString& error)
{
	mWorkingText->setText(QString("Error: ") + error);
	mWorkingIcon->setPixmap(QPixmap(":/icons/error.png"));
	setCurrentWidget(mWorkingPane);
}

void Editor::openFileSuccessful(File* file)
{
	mFile = file;
	mDocument->setPlainText(file->getData());
	setCurrentWidget(mEditor);
	connect(mDocument, SIGNAL(contentsChange(int,int,int)), this, SLOT(docChanged(int,int,int)));
}

void Editor::docChanged(int position, int charsRemoved, int charsAdded)
{
	QByteArray added = "";
	for (int i = 0; i < charsAdded; i++)
		added += mDocument->characterAt(i + position);

	mFile->changeDocument(position, charsRemoved, added);
}

