#include <QLabel>
#include <QPushButton>
#include <QToolButton>

#include "editorwarningbar.h"

EditorWarningBar::EditorWarningBar( QWidget* parent, const QPixmap& icon, const QString& message ) : QWidget( parent ) {
	mLayout = new QHBoxLayout( this );

	QPalette p( palette() );
	p.setColor( QPalette::Background, p.color( QPalette::ToolTipBase ) );
	setPalette( p );
	setAutoFillBackground( true );


	QLabel* iconLabel = new QLabel( this );
	iconLabel->setPixmap( icon );
	mLayout->addWidget( iconLabel );

	QLabel* textLabel = new QLabel( this );
	textLabel->setText( message );
	mLayout->addWidget( textLabel );
	mLayout->setMargin( 3 );

	mLayout->addSpacerItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding ) );
}

void EditorWarningBar::addButton( const QString &label, QObject *callbackTarget, const char *callbackMethod ) {
	QPushButton* button = new QPushButton( this );
	button->setText( label );
	mLayout->addWidget( button );

	connect( button, SIGNAL( clicked() ), callbackTarget, callbackMethod );
}

void EditorWarningBar::addCloseButton() {
	QToolButton* closeButton = new QToolButton( this );
	closeButton->setIcon( QIcon( ":/icons/cross.png" ) );
	mLayout->addWidget( closeButton );

	connect( closeButton, SIGNAL( clicked() ), this, SLOT( closeAndDestroy() ) );
}

void EditorWarningBar::closeAndDestroy() {
	hide();
	deleteLater();
}
