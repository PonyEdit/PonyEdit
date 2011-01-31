#include <QTextDocument>
#include <QDebug>

#include "htmlpreview.h"
#include "ui_htmlpreview.h"

#include "main/globaldispatcher.h"
#include "main/mainwindow.h"
#include "editor/editor.h"

HTMLPreview::HTMLPreview(MainWindow *parent) :
    QWidget(parent),
    ui(new Ui::HTMLPreview)
{
	mParent = parent;

    ui->setupUi(this);

	ui->refreshFrom->addItem(tr("Current File"));
	ui->refreshFrom->addItem(tr("URL"));

	ui->refreshWhen->addItem(tr("As You Type"));
	ui->refreshWhen->addItem(tr("After Save"));
	ui->refreshWhen->addItem(tr("Manually"));

#ifdef Q_OS_MAC
	ui->refreshButton->setMinimumHeight(32);
#endif

	connect(gDispatcher, SIGNAL(selectFile(BaseFile*)), this, SLOT(fileSelected(BaseFile*)));

	connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(manualRefresh()));

	Editor *current = mParent->getCurrentEditor();
	if(current)
	{
		BaseFile *file = current->getFile();
		QTextDocument *doc = file->getTextDocument();

		connect(file, SIGNAL(unsavedStatusChanged()), this, SLOT(fileSaved()));
		connect(doc, SIGNAL(contentsChanged()), this, SLOT(fileChanged()));

		manualRefresh();
	}
}

HTMLPreview::~HTMLPreview()
{
    delete ui;
}

void HTMLPreview::fileSelected(BaseFile *file)
{
	if(!file)
		return;

	QTextDocument *doc = file->getTextDocument();

	connect(file, SIGNAL(unsavedStatusChanged()), this, SLOT(fileSaved()));
	connect(doc, SIGNAL(contentsChanged()), this, SLOT(fileChanged()));
}

void HTMLPreview::fileSaved()
{
	if(ui->refreshWhen->currentIndex() != 1)
		return;

	BaseFile *file = dynamic_cast<BaseFile*>(sender());

	if(file->hasUnsavedChanges())
		return;

	switch(ui->refreshFrom->currentIndex())
	{
		case 0:
			displayHTML(file->getTextDocument()->toPlainText());
			break;
		case 1:
			displayURL();
			break;
	}
}

void HTMLPreview::fileChanged()
{
	if(ui->refreshWhen->currentIndex() != 0)
		return;

	QTextDocument *doc = dynamic_cast<QTextDocument*>(sender());

	if(ui->refreshFrom->currentIndex() == 0)
		displayHTML(doc->toPlainText());
}

void HTMLPreview::manualRefresh()
{
	Editor *current = mParent->getCurrentEditor();

	BaseFile *file = NULL;
	if(current)
		file = current->getFile();

	switch(ui->refreshFrom->currentIndex())
	{
		case 0:
			if(file)
				displayHTML(file->getTextDocument()->toPlainText());
			break;
		case 1:
			displayURL();
			break;
	}

}

void HTMLPreview::displayHTML(QString html)
{
	ui->webView->setHtml(html);
}

void HTMLPreview::displayURL()
{
	QUrl url = QUrl(ui->url->text());
	ui->webView->load(url);
}
