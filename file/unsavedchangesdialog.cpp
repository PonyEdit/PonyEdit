#include "unsavedchangesdialog.h"

#include "QsLog.h"
#include "basefile.h"
#include "filedialog.h"
#include "openfilemanager.h"
#include "openfiletreeview.h"
#include <QDebug>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

UnsavedChangesDialog::UnsavedChangesDialog( const QList< BaseFile * > &files, bool closeFilesOnDiscard )
    : QDialog( 0 ), mCloseFilesOnDiscard( closeFilesOnDiscard ) {
	QVBoxLayout *layout = new QVBoxLayout( this );
	setLayout( layout );

	QLabel *label = new QLabel( this );
	label->setText( "The following files have unsaved changes: " );
	layout->addWidget( label );

	mTreeView = new OpenFileTreeView( this, OpenFileTreeView::MultiSelect | OpenFileTreeView::UnsavedOnly, &files );
	layout->addWidget( mTreeView );
	mTreeView->expandAll();
	mTreeView->setMinimumWidth( 250 );
	mTreeView->setMinimumHeight( 200 );
	mTreeView->selectAll();

	mButtonBox = new QDialogButtonBox( this );
	mButtonBox->setStandardButtons( QDialogButtonBox::Save | QDialogButtonBox::Discard | QDialogButtonBox::Cancel );
	mButtonBox->button( QDialogButtonBox::Save )->setDefault( true );
	layout->addWidget( mButtonBox );

	foreach ( BaseFile *file, files )
		connect( file, SIGNAL( unsavedStatusChanged() ), this, SLOT( fileStateChanged() ) );

	connect( mTreeView->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), this, SLOT( selectionChanged( QItemSelection, QItemSelection ) ) );
	connect( mButtonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
	connect( mButtonBox, SIGNAL( clicked( QAbstractButton * ) ), this, SLOT( buttonClicked( QAbstractButton * ) ) );
	connect( &gOpenFileManager, SIGNAL( fileClosed( BaseFile * ) ), this, SLOT( fileClosed( BaseFile * ) ) );
}

UnsavedChangesDialog::~UnsavedChangesDialog() {
}

void UnsavedChangesDialog::buttonClicked( QAbstractButton *button ) {
	QList< BaseFile * > selectedFiles = mTreeView->getSelectedFiles();
	if ( button == (QAbstractButton *) mButtonBox->button( QDialogButtonBox::Save ) ) {
		//	Save
		foreach ( BaseFile *file, selectedFiles ) {
			if ( file->getLocation().getProtocol() == Location::Unsaved ) {
				FileDialog dlg( this, true );
				if ( dlg.exec() ) {
					Location loc = dlg.getNewLocation();
					loc.getFile()->newFile( file->getContent() );
				}
			} else {
				try {
					file->save();
				} catch ( QString & /*e*/ ) {
					QLOG_ERROR() << "Unexpected throw while saving file" << file->getLocation().getLabel();
				}
			}
		}
	} else if ( button == (QAbstractButton *) mButtonBox->button( QDialogButtonBox::Discard ) ) {
		//	Discard
		foreach ( BaseFile *file, selectedFiles ) {
			try {
				if ( mCloseFilesOnDiscard ) {
					if ( file->canClose() )
						file->close();
				} else
					fileClosed( file );
			} catch ( QString & /*e*/ ) {
				QLOG_ERROR() << "Unexpected throw while discarding file" << file->getLocation().getLabel();
			}
		}
	}
}

void UnsavedChangesDialog::selectionChanged( QItemSelection /* before */, QItemSelection /* after */ ) {
	bool itemsSelected = mTreeView->selectionModel()->selectedRows().count() > 0;
	mButtonBox->button( QDialogButtonBox::Save )->setEnabled( itemsSelected );
	mButtonBox->button( QDialogButtonBox::Discard )->setEnabled( itemsSelected );
}

void UnsavedChangesDialog::fileStateChanged() {
	BaseFile *file = static_cast< BaseFile * >( QObject::sender() );
	if ( !file->hasUnsavedChanges() ) {
		BaseFile::OpenStatus status = file->getOpenStatus();

		if ( status != BaseFile::Closed && status != BaseFile::Closing ) {
			try {
				if ( file->canClose() )
					file->close();
			} catch ( QString & /*e*/ ) {
				QLOG_ERROR() << "Unexpected throw while closing saved file" << file->getLocation().getLabel();
			}
		}
	}
}

void UnsavedChangesDialog::fileClosed( BaseFile *file ) {
	mTreeView->removeFile( file );
	if ( mTreeView->model()->rowCount() == 0 )
		accept();
}
