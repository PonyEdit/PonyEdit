#include "gotolinedialog.h"

HIDE_COMPILE_WARNINGS

#include "ui_gotolinedialog.h"

UNHIDE_COMPILE_WARNINGS

GotoLineDialog::GotoLineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GotoLineDialog),
    mLineNumber(0)
{
	ui->setupUi(this);

	connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
	connect(ui->gotoButton, SIGNAL(clicked()), this, SLOT(accept()));
}

GotoLineDialog::~GotoLineDialog()
{
    delete ui;
}

void GotoLineDialog::accept()
{
	QString line = ui->line->text();
	mLineNumber = line.toInt();

	QDialog::accept();
}

int GotoLineDialog::lineNumber()
{
	return mLineNumber;
}
