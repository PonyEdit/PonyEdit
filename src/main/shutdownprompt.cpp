#include <QStyle>

#include "shutdownprompt.h"
#include "ui_shutdownprompt.h"

#include "options/options.h"

ShutdownPrompt::ShutdownPrompt( QWidget *parent ) :
	QDialog( parent ),
	ui( new Ui::ShutdownPrompt ) {
	ui->setupUi( this );

	QStyle *style = this->style();
	int iconSize = style->pixelMetric( QStyle::PM_MessageBoxIconSize, nullptr, this );

	QIcon tmpIcon = style->standardIcon( QStyle::SP_MessageBoxQuestion, nullptr, this );

	ui->iconLabel->setPixmap( tmpIcon.pixmap( iconSize, iconSize ) );

	connect( ui->remember, SIGNAL( clicked() ), this, SLOT( remember() ) );
	connect( ui->dontRemember, SIGNAL( clicked() ), this, SLOT( dontRemember() ) );
}

ShutdownPrompt::~ShutdownPrompt() {
	delete ui;
}

void ShutdownPrompt::remember() {
	if ( ui->dontPrompt->isChecked() ) {
		Options::ShutdownPrompt = false;
	}

	Options::StartupAction = Options::ReopenFiles;

	accept();
}

void ShutdownPrompt::dontRemember() {
	if ( ui->dontPrompt->isChecked() ) {
		Options::ShutdownPrompt = false;
	}

	Options::StartupAction = Options::NoFiles;

	accept();
}
