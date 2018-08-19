#include <QDebug>
#include <QStringList>
#include "fontoptionswidget.h"
#include "options.h"
#include "ui_fontoptionswidget.h"

FontOptionsWidget::FontOptionsWidget( QWidget *parent ) :
	OptionsDialogPage( parent ),
	ui( new Ui::fontoptionswidget ) {
	ui->setupUi( this );

	//
	// Populate font families list
	//

	QStringList families = mFontDatabase.families();
	foreach ( QString family, families ) {
		ui->fontCombo->addItem( family );
	}

	QStringList fontFamilies = QFont::substitutes( Options::EditorFont->family() );
	fontFamilies.push_front( Options::EditorFont->family() );
	foreach ( QString family, fontFamilies ) {
		int index = ui->fontCombo->findText( family, Qt::MatchFixedString );
		if ( index > 0 ) {
			ui->fontCombo->setCurrentIndex( index );
			break;
		}
	}

	//
	// Populate standard sizes list
	//

	QList< int > sizes = QFontDatabase::standardSizes();
	foreach ( int size, sizes ) {
		ui->sizeCombo->addItem( QString::number( size ) );
	}

	int index = ui->sizeCombo->findText( QString::number( Options::EditorFont->pointSize() ) );
	ui->sizeCombo->setCurrentIndex( index );
}

FontOptionsWidget::~FontOptionsWidget() {
	delete ui;
}

void FontOptionsWidget::apply() {
	bool ok;
	int size = ui->sizeCombo->currentText().toInt( &ok );
	if ( ! ok ) {
		size = 12;
	}

	Options::EditorFont = new QFont( mFontDatabase.font( ui->fontCombo->currentText(), "", size ) );
}
