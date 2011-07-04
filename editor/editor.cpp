#include "editor.h"
#include <QGridLayout>
#include <QSpacerItem>
#include <QTextCursor>
#include <QDebug>

#include "file/basefile.h"
#include "syntax/syntaxhighlighter.h"
#include "syntax/syntaxdefinition.h"
#include "syntax/syntaxdefmanager.h"
#include "options/options.h"
#include "main/globaldispatcher.h"
#include "editorwarningbar.h"
#include "file/openfilemanager.h"

Editor::Editor(BaseFile* file) : QStackedWidget()
{
	mReadOnlyWarning = NULL;
	mFirstOpen = true;

	mEditorPane = new QWidget(this);
	mEditorPaneLayout = new QVBoxLayout(mEditorPane);
	mEditorPaneLayout->setSpacing(0);
	mEditorPaneLayout->setMargin(0);
	mEditor = new CodeEditor(file, this);
	mEditorPaneLayout->addWidget(mEditor);
	addWidget(mEditorPane);

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

	connect(gDispatcher, SIGNAL(optionsChanged()), this, SLOT(applyOptions()));
	applyOptions();
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

				if (mFile->isReadOnly())
				{
					setReadOnly(true);
					showReadOnlyWarning();
				}
			}

		case BaseFile::Disconnected:
		case BaseFile::Reconnecting:
		case BaseFile::Repairing:
			setCurrentWidget(mEditorPane);
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
	if (mFile->canClose())
		mFile->close();
}

int Editor::find(const QString &text, bool backwards, bool caseSensitive, bool useRegexp, bool loop)
{
	QTextCursor found = internalFind(text, backwards, caseSensitive, useRegexp, loop);

	if(found.isNull())
		return 0;
	else
		return 1;
}

QTextCursor Editor::internalFind(const QString& text, bool backwards, bool caseSensitive, bool useRegexp, bool loop)
{
	if(backwards && mEditor->textCursor().selectionStart() == 0 && !loop)
		return QTextCursor();

	QTextDocument *doc = mEditor->document();

	QString content = mEditor->toPlainText();

	QTextCursor currentCursor = mEditor->textCursor();

	QTextDocument::FindFlags flags = 0;
	if(caseSensitive)
		flags |= QTextDocument::FindCaseSensitively;
	if(backwards)
		flags |= QTextDocument::FindBackward;

	QTextCursor newSelection;

	if(useRegexp)
	{
		QRegExp regexp(text, (Qt::CaseSensitivity)(caseSensitive)?(Qt::CaseSensitive):(Qt::CaseInsensitive));

		if(!content.contains(regexp))
			return QTextCursor();

		newSelection = doc->find(regexp, currentCursor, flags);
	}
	else
	{
		if(!content.contains(text, (Qt::CaseSensitivity)(caseSensitive)?(Qt::CaseSensitive):(Qt::CaseInsensitive)))
			return QTextCursor();

		newSelection = doc->find(text, currentCursor, flags);
	}

	if(!newSelection.isNull())
	{
		mEditor->setTextCursor(newSelection);
	}
	else if(loop)
	{
		if(backwards)
		{
			currentCursor.movePosition(QTextCursor::End);
			mEditor->setTextCursor(currentCursor);
		}
		else
			gotoLine(1);

		newSelection = internalFind(text, backwards, caseSensitive, useRegexp, false);
	}

	return newSelection;
}

int Editor::replace(const QString &findText, const QString &replaceText, bool caseSensitive, bool useRegex, bool all)
{
	if(findText.length() <= 0)
		return 0;

	QString content = mEditor->toPlainText();

	QRegExp regexp(findText, (Qt::CaseSensitivity)(caseSensitive)?(Qt::CaseSensitive):(Qt::CaseInsensitive));

	if(useRegex)
	{
		if(!content.contains(regexp))
			return 0;
	}
	else
	{
		if(!content.contains(findText, (Qt::CaseSensitivity)(caseSensitive)?(Qt::CaseSensitive):(Qt::CaseInsensitive)))
			return 0;
	}

	QTextCursor found = internalFind(findText, false, caseSensitive, useRegex);

	if(all)
	{
		QTextCursor editBlock = found;

		editBlock.movePosition(QTextCursor::Start);
		editBlock.beginEditBlock();
		mEditor->setTextCursor(editBlock);

		while(!found.isNull())
		{
			found.insertText(replaceText);

			found = internalFind(findText, false, caseSensitive, useRegex, false);
		}

		editBlock.endEditBlock();
	}
	else
	{
		found.insertText(replaceText);
		mEditor->setTextCursor(found);
	}

	return 1;
}

void Editor::gotoLine(int lineNumber)
{
	if(lineNumber < 1)
		lineNumber = 1;

	QTextCursor cursor = mEditor->textCursor();

	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, lineNumber - 1);

	mEditor->setTextCursor(cursor);
}

void Editor::gotoEnd()
{
	QTextCursor cursor = mEditor->textCursor();

	cursor.movePosition(QTextCursor::End);

	mEditor->setTextCursor(cursor);
}

void Editor::fileClosed()
{
	delete this;
}

void Editor::setFocus()
{
	mEditor->setFocus();
}

bool Editor::hasFocus()
{
	return mEditor->hasFocus();
}

void Editor::applyOptions()
{
	QFont font = Options::EditorFont;

	QFontMetrics fontMetrics(font);
	int characterWidth = fontMetrics.width("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM") / 40;

	mEditor->updateFont();
	mEditor->setTabStopWidth(Options::TabStopWidth * characterWidth);
	mEditor->setLineWrapMode(Options::WordWrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
}

void Editor::setReadOnly(bool readOnly)
{
	mEditor->setReadOnly(readOnly);

	if (!readOnly && mReadOnlyWarning)
	{
		delete mReadOnlyWarning;
		mReadOnlyWarning = NULL;
	}
}

void Editor::showReadOnlyWarning()
{
	if (mReadOnlyWarning) delete mReadOnlyWarning;

	mReadOnlyWarning = new EditorWarningBar(this, QPixmap(":/icons/warning.png"),
		tr("You do not have write access to this file. It has been opened in read-only mode."));

	if (mFile->getLocation().canSudo() && !mFile->getLocation().isSudo())
		mReadOnlyWarning->addButton(tr("Sudo and Edit"), this, SLOT(sudo()));

	mReadOnlyWarning->addButton(tr("Edit Anyway"), this, SLOT(enableEditing()));
	mEditorPaneLayout->insertWidget(0, mReadOnlyWarning);
}

void Editor::sudo()
{
	mFile->sudo();
	gOpenFileManager.reregisterFile(mFile);
	setReadOnly(false);
}
