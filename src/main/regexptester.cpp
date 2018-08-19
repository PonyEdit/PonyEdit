#include <QDebug>

#include "globaldispatcher.h"
#include "options/options.h"
#include "regexptester.h"
#include "ui_regexptester.h"

RegExpTester::RegExpTester( QWidget *parent ) :
	QWidget( parent ),
	ui( new Ui::RegExpTester ) {
	ui->setupUi( this );

	ui->regexpType->addItem( tr( "Perl" ), QVariant( static_cast< int >( QRegExp::RegExp ) ) );
	ui->regexpType->addItem( tr( "Greedy Perl" ), QVariant( static_cast< int >( QRegExp::RegExp2 ) ) );
	ui->regexpType->addItem( tr( "W3C XML Schema 1.1" ),
	                         QVariant( static_cast< int >( QRegExp::W3CXmlSchema11 ) ) );

	connect( ui->regexpType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateResult()) );
	connect( ui->regexp, SIGNAL(textChanged(QString)), this, SLOT(updateResult()) );
	connect( ui->testData, SIGNAL(textChanged()), this, SLOT(updateResult()) );
	connect( ui->caseSensitive, SIGNAL(stateChanged(int)), this, SLOT(updateResult()) );
	connect( gDispatcher, SIGNAL(optionsChanged()), this, SLOT(applySettings()) );

	mUpdating = false;

	mCaptureColors << 0xFFD56B
	               << 0x8C9DFF
	               << 0xFF8C8C
	               << 0x68FFF4
	               << 0xEA84FF
	               << 0x90FF8E
	               << 0xFFAB96;

	applySettings();
	updateResult();
}

RegExpTester::~RegExpTester() {
	delete ui;
}

void RegExpTester::applySettings() {
	ui->regexp->setFont( *Options::EditorFont );
	ui->testData->setFont( *Options::EditorFont );
}

void RegExpTester::takeFocus( const QString &regExp ) {
	if ( ! regExp.isEmpty() ) {
		ui->regexp->setText( regExp );
		ui->testData->setFocus();
		ui->testData->selectAll();
	} else {
		ui->regexp->setFocus();
		ui->regexp->selectAll();
	}
}

void RegExpTester::updateResult() {
	if ( mUpdating ) {
		return;
	}
	mUpdating = true;

	QRegExp regExp = QRegExp( ui->regexp->text(),
	                          ui->caseSensitive->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive,
	                          static_cast< QRegExp::PatternSyntax >( ui->regexpType->itemData( ui->regexpType->
	                                                                                           currentIndex() ).
	                                                                 toInt() ) );

	QString data = ui->testData->toPlainText();
	ui->captures->clear();

	int index = regExp.indexIn( data );
	if ( index > -1 ) {
		int length = regExp.matchedLength();
		ui->matchedText->setText( QString( "\"" ) + data.mid( index, length > 100 ? 100 : length ) + "\"" );



		QTextCursor cursor( ui->testData->document() );
		cursor.movePosition( QTextCursor::End, QTextCursor::KeepAnchor );
		cursor.setCharFormat( QTextCharFormat() );

		int colorCount = 0;
		for ( int i = 1; i <= regExp.captureCount(); i++ ) {
			quint32 color         = mCaptureColors[ colorCount ];
			QListWidgetItem *item = new QListWidgetItem( regExp.cap( i ) );

			item->setBackgroundColor( QColor( color ) );
			ui->captures->addItem( item );

			QTextCharFormat f;
			f.setBackground( QBrush( QColor( color ) ) );
			cursor.setPosition( regExp.pos( i ), QTextCursor::MoveAnchor );
			cursor.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor, regExp.cap( i ).length() );
			cursor.setCharFormat( f );

			colorCount++;
			if ( colorCount > mCaptureColors.length() ) {
				colorCount = 0;
			}
		}
	} else {
		ui->matchedText->setText( "- No Matches -" );
	}

	mUpdating = false;
}
