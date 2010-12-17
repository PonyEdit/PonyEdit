#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QStringList>
#include <QUrl>
#include <QList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>


#include "optionsdialog.h"
#include "ui_optionsdialog.h"

const char* OptionsDialog::sOptionsStrings[] = { "Editor" };

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

	mNetworkManager = new QNetworkAccessManager(this);

	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
	connect(this, SIGNAL(accepted()), this, SLOT(saveOptions()));
	connect(ui->optionList,SIGNAL(currentRowChanged(int)),this,SLOT(updateSelectedOption(int)));

	for(int ii = 0; ii < NumOptions; ii++)
	{
		ui->optionList->addItem(sOptionsStrings[ii]);
	}

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
}
