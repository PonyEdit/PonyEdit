#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QList>
#include <QStringList>

#include "main/globaldispatcher.h"
#include "ssh2/serverconfigwidget.h"
#include "sshserveroptionswidget.h"

#include "advancedoptionswidget.h"
#include "editoroptionswidget.h"
#include "fontoptionswidget.h"
#include "options.h"
#include "optionsdialog.h"
#include "startupoptionswidget.h"
#include "ui_optionsdialog.h"

OptionsDialog::OptionsDialog( QWidget *parent )
    : QDialog( parent ), ui( new Ui::OptionsDialog ) {
	ui->setupUi( this );

	connect( ui->buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
	connect( ui->buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
	connect( ui->buttonBox, SIGNAL( clicked( QAbstractButton * ) ), this, SLOT( buttonClicked( QAbstractButton * ) ) );
	connect( this, SIGNAL( accepted() ), this, SLOT( saveOptions() ) );

	addPage( ui->editorButton, new EditorOptionsWidget( this ) );
	addPage( ui->serversButton, new SshServerOptionsWidget( this ) );
	addPage( ui->appearanceButton, new FontOptionsWidget( this ) );
	addPage( ui->startupButton, new StartupOptionsWidget( this ) );
	addPage( ui->loggingButton, new AdvancedOptionsWidget( this ) );

	ui->editorButton->setChecked( true );
	pageClicked( ui->editorButton );
}

OptionsDialog::~OptionsDialog() {
	delete ui;
}

void OptionsDialog::buttonClicked( QAbstractButton *button ) {
	if ( ui->buttonBox->buttonRole( button ) == QDialogButtonBox::ApplyRole )
		saveOptions();
}

void OptionsDialog::saveOptions() {
	foreach ( OptionsDialogPage *page, mPages )
		page->apply();

	::Options::save();

	gDispatcher->emitOptionsChanged();
}

void OptionsDialog::pageClicked() {
	QToolButton *button = (QToolButton *) QObject::sender();
	pageClicked( button );
}

void OptionsDialog::pageClicked( QToolButton *button ) {
	OptionsDialogPage *page = mPageMap.value( button, NULL );
	if ( page ) {
		ui->stackedWidget->setCurrentWidget( page );
		ui->optionLabel->setText( button->text() );
	}
}

void OptionsDialog::addPage( QToolButton *button, OptionsDialogPage *page ) {
	mPageMap.insert( button, page );
	connect( button, SIGNAL( clicked() ), this, SLOT( pageClicked() ) );
	mPages.append( page );
	ui->stackedWidget->addWidget( page );
}
