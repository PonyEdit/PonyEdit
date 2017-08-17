#include "file/basefile.h"
#include "file/location.h"
#include "file/openfilemanager.h"
#include "options.h"
#include "startupoptionswidget.h"
#include "ui_startupoptionswidget.h"

StartupOptionsWidget::StartupOptionsWidget( QWidget *parent ) :
	OptionsDialogPage( parent ),
	ui( new Ui::StartupOptionsWidget ) {
	ui->setupUi( this );

	switch ( Options::StartupAction ) {
	case Options::BlankFile:
		ui->blankFile->setChecked( true );
		break;

	case Options::ReopenFiles:
		ui->reopenFiles->setChecked( true );
		break;

	case Options::SetFiles:
		ui->setFiles->setChecked( true );
		break;

	case Options::NoFiles:
	default:
		ui->noFiles->setChecked( true );
	}

	if ( Options::StartupAction == Options::SetFiles ) {
		ui->fileList->setEnabled( true );

		foreach ( QString file, Options::StartupFiles ) {
			ui->fileList->appendPlainText( file.trimmed() );
		}
	} else {
		ui->fileList->setEnabled( false );
	}

	ui->confirmOnExit->setChecked( Options::ShutdownPrompt );

	connect( ui->setFiles, SIGNAL( toggled( bool ) ), this, SLOT( SetFilesToggled( bool ) ) );
	connect( ui->currentFiles, SIGNAL( clicked() ), this, SLOT( SetFilesToCurrent() ) );
}

StartupOptionsWidget::~StartupOptionsWidget() {
	delete ui;
}

void StartupOptionsWidget::apply() {
	if ( ui->noFiles->isChecked() ) {
		Options::StartupFiles.clear();
		Options::StartupFilesLineNo.clear();

		Options::StartupAction = Options::NoFiles;
	} else if ( ui->blankFile->isChecked() ) {
		Options::StartupFiles.clear();
		Options::StartupFilesLineNo.clear();

		Options::StartupAction = Options::BlankFile;
	} else if ( ui->reopenFiles->isChecked() ) {
		Options::StartupAction = Options::ReopenFiles;
	} else if ( ui->setFiles->isChecked() ) {
		Options::StartupFiles.clear();
		Options::StartupFilesLineNo.clear();

		Options::StartupAction = Options::SetFiles;
		Options::StartupFiles = ui->fileList->toPlainText().split( QRegExp( "(\r|\n)+" ) );
		for ( int ii = 0; ii < Options::StartupFiles.length(); ii++ ) {
			Options::StartupFilesLineNo.append( 1 );
		}
	}

	Options::ShutdownPrompt = ui->confirmOnExit->isChecked();
}

void StartupOptionsWidget::SetFilesToggled( bool checked ) {
	ui->fileList->setEnabled( checked );
}

void StartupOptionsWidget::SetFilesToCurrent() {
	if ( ! ui->setFiles->isChecked() ) {
		return;
	}

	QList< BaseFile * > files = gOpenFileManager.getOpenFiles();

	ui->fileList->clear();
	foreach ( BaseFile *file, files ) {
		Location loc = file->getLocation();
		ui->fileList->appendPlainText( loc.getDisplayPath() );
	}
}
