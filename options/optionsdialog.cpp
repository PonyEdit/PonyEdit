#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QStringList>
#include <QList>

#include "ssh/serverconfigwidget.h"
#include "sshserveroptionswidget.h"
#include "main/globaldispatcher.h"

#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "options.h"
#include "fontoptionswidget.h"

QString OptionsDialog::sOptionsStrings[] = { tr("Editor"), tr("SSH Servers"), tr("Fonts & Colors") };

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
	connect(this, SIGNAL(accepted()), this, SLOT(saveOptions()));
	connect(ui->optionList,SIGNAL(currentRowChanged(int)),this,SLOT(updateSelectedOption(int)));

	for(int ii = 0; ii < NumOptions; ii++)
	{
		ui->optionList->addItem(sOptionsStrings[ii]);
	}

	addPage(new SshServerOptionsWidget(this));
	addPage(new FontOptionsWidget(this));

	ui->optionList->setCurrentRow(0);
	updateSelectedOption(0);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::updateSelectedOption(int newOption)
{
	ui->stackedWidget->setCurrentIndex(newOption);
	ui->optionLabel->setText(sOptionsStrings[newOption]);
}

void OptionsDialog::buttonClicked(QAbstractButton *button)
{
	if(ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
		saveOptions();
}

void OptionsDialog::saveOptions()
{
	foreach (OptionsDialogPage* page, mPages)
		page->apply();

	::Options::save();

	gDispatcher->emitOptionsChanged();
}

void OptionsDialog::addPage(OptionsDialogPage *page)
{
	mPages.append(page);
	ui->stackedWidget->addWidget(page);
}
