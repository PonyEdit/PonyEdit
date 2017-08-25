#include "hostkeydlg.h"
#include "ui_hostkeydlg.h"

HostKeyDlg::HostKeyDlg( QWidget *parent ) :
	ThreadCrossingDialog( parent ),
	ui( new Ui::HostKeyDlg ) {
	ui->setupUi( this );
}

HostKeyDlg::~HostKeyDlg() {
	delete ui;
}

void HostKeyDlg::setOptions( const QVariantMap &options ) {
	setWindowTitle( options.value( "title" ).toString() );
	ui->body->setText( options.value( "body" ).toString() );
}
