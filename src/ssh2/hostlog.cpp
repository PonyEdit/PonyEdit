#include "hostlog.h"
#include "sshhost.h"
#include "ui_hostlog.h"

HostLog::HostLog( SshHost *host ) :
	QWidget( nullptr ),
	ui( new Ui::HostLog ) {
	ui->setupUi( this );
	ui->textBox->setPlainText( host->getLog().join( "\n" ) );
	connect( host, SIGNAL( newLogLine( QString ) ), this, SLOT( newLogLine( QString ) ) );

	setWindowTitle( "Log: " + host->getName() );
}

HostLog::~HostLog() {
	delete ui;
}

void HostLog::closeEvent( QCloseEvent * ) {
	this->deleteLater();
}

void HostLog::newLogLine( const QString &line ) {
	ui->textBox->appendPlainText( line );
}
