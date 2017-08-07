#include "favoritelocationdialog.h"
#include "ui_favoritelocationdialog.h"


FavoriteLocationDialog::FavoriteLocationDialog( QWidget *parent, Location::Favorite* favorite ) : QDialog( parent ),
	ui( new Ui::FavoriteLocationDialog ) {
	ui->setupUi( this );

	mFavorite = favorite;
	ui->name->setText( mFavorite->name );
	ui->path->setText( mFavorite->path );

	ui->name->setFocus();
	ui->name->selectAll();

	connect( this, SIGNAL( accepted() ), this, SLOT( acceptHandler() ) );
}

FavoriteLocationDialog::~FavoriteLocationDialog() {
	delete ui;
}

void FavoriteLocationDialog::acceptHandler() {
	QString name = ui->name->text();
	if ( ! name.isEmpty() ) {
		mFavorite->name = name;
	}
}
