#include "gotolinedialog.h"
#include "ui_gotolinedialog.h"

GotoLineDialog::GotoLineDialog( QWidget *parent ) :
	QDialog( parent ),
	ui( new Ui::GotoLineDialog ) {
	ui->setupUi( this );

	mLineNumber = 0;

	connect( ui->cancelButton, SIGNAL( clicked() ), this, SLOT( close() ) );
	connect( ui->gotoButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
}

GotoLineDialog::~GotoLineDialog() {
	delete ui;
}

void GotoLineDialog::accept() {
	QString line = ui->line->text();
	mLineNumber = line.toInt();

	QDialog::accept();
}

int GotoLineDialog::lineNumber() {
	return mLineNumber;
}
