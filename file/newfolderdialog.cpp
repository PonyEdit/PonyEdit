#include <QMessageBox>
#include <QPushButton>

#include "newfolderdialog.h"
#include "ssh2/slaverequest.h"
#include "tools/callback.h"
#include "ui_newfolderdialog.h"

NewFolderDialog::NewFolderDialog( QWidget *parent, const Location& parentLocation ) :
	QDialog( parent ),
	ui( new Ui::NewFolderDialog ) {
	mParentLocation = parentLocation;
	ui->setupUi( this );
	ui->folderName->setFocus();
}

NewFolderDialog::~NewFolderDialog() {
	delete ui;
}

void NewFolderDialog::accept() {
	attempt( false );
}

void NewFolderDialog::attempt( bool sudo ) {
	ui->buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
	ui->folderName->setEnabled( false );

	Location loc = ( sudo ? mParentLocation.getSudoLocation() : mParentLocation );
	loc.createNewDirectory( ui->folderName->text(),
	                        Callback( this,
	                                  SLOT( createSuccess( QVariantMap ) ),
	                                  SLOT( createFailure( QString, int ) ) ) );
}

void NewFolderDialog::createSuccess( QVariantMap /*result*/ ) {
	QDialog::accept();
}

void NewFolderDialog::createFailure( QString error, int flags ) {
	QMessageBox msgbox;
	msgbox.setWindowTitle( "Failed to create remote directory" );
	msgbox.setText( error );
	msgbox.setStandardButtons( QMessageBox::Cancel );

	QPushButton* sudoButton = NULL;
	if ( flags & SlaveRequest::PermissionError ) {
		sudoButton = msgbox.addButton( "Sudo and Try Again", QMessageBox::ActionRole );
	}

	msgbox.exec();

	if ( sudoButton != NULL && msgbox.clickedButton() == sudoButton ) {
		attempt( true );
	} else {
		reject();
	}
}
