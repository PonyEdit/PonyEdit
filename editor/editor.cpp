#include "editor.h"
#include <QGridLayout>
#include <QSpacerItem>
#include <QTextCursor>
#include <QDebug>

#include "file/basefile.h"
#include "syntax/syntaxhighlighter.h"
#include "syntax/syntaxdefinition.h"
#include "syntax/syntaxdefmanager.h"

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

void Editor::close()
{
	mFile->close();
}

int Editor::find(const QString& text, bool backwards, bool caseSensitive, bool useRegexp)
{
	int foundCount = 0, lastPos = 0, found = 0, length = 0;

	QString content = mEditor->toPlainText();

	if(backwards)
		lastPos = mEditor->textCursor().selectionStart() - 1;
	else
		lastPos = mEditor->textCursor().selectionEnd();

	if(useRegexp)
	{
		QRegExp regexp(text, (Qt::CaseSensitivity)(caseSensitive)?(Qt::CaseSensitive):(Qt::CaseInsensitive));

		if(backwards)
			found = regexp.lastIndexIn(content, lastPos);
		else
			found = regexp.indexIn(content, lastPos);

		length = regexp.matchedLength();
		foundCount = content.count(regexp);
	}
	else
	{
		if(backwards)
			found = content.lastIndexOf(text, lastPos, (Qt::CaseSensitivity)(caseSensitive)?(Qt::CaseSensitive):(Qt::CaseInsensitive));
		else
			found = content.indexOf(text, lastPos, (Qt::CaseSensitivity)(caseSensitive)?(Qt::CaseSensitive):(Qt::CaseInsensitive));

		length = text.length();
		foundCount = content.count(text, (Qt::CaseSensitivity)(caseSensitive)?(Qt::CaseSensitive):(Qt::CaseInsensitive));
	}

	if(found >= 0)
	{
		QTextCursor newSelection = mEditor->textCursor();
		newSelection.setPosition(found);
		newSelection.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, length);

		mEditor->setTextCursor(newSelection);
	}

	return (found >= 0)?(foundCount):(0);
}

int Editor::replace(const QString &findText, const QString &replaceText, bool caseSensitive, bool useRegex, bool all)
{
	if(findText.length() <= 0)
		return 0;

	QString content = mEditor->toPlainText();

	QRegExp regexp(findText, (Qt::CaseSensitivity)(caseSensitive)?(Qt::CaseSensitive):(Qt::CaseInsensitive));

	if(all)
	{
		if(useRegex)
			content.replace(regexp, replaceText);
		else
			content.replace(findText, replaceText);
	}
	else
	{
		QTextCursor selection = mEditor->textCursor();

		if(selection.selectionStart() >= selection.selectionEnd())
			return 0;

		QString selectedText = content.mid(selection.selectionStart(), selection.selectionEnd() - selection.selectionStart());
		QString formattedText = replaceText;

		if(useRegex)
		{
			if(!regexp.exactMatch(selectedText))
				return 0;

			QStringList caps = regexp.capturedTexts();

			for(int ii = 1; ii < caps.size(); ii++)
				formattedText.replace(QString("\\%1").arg(ii), caps[ii]);
		}
		else
		{
			if(selectedText != findText)
				return 0;
		}

		content.remove(selection.selectionStart(), selection.selectionEnd() - selection.selectionStart());
		content.insert(selection.selectionStart(), formattedText);
	}

	mEditor->selectAll();
	mEditor->insertPlainText(content);

	return 1;
}

void Editor::fileClosed()
{
	delete this;
}

void Editor::setFocus()
{
	mEditor->setFocus();
}
