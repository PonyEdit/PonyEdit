#include "threadcrossingdialog.h"

ThreadCrossingDialog::ThreadCrossingDialog( QWidget *parent ) :
	QDialog( parent ) {}

QVariantMap ThreadCrossingDialog::getResult() {
	QVariantMap result;
	result.insert( "accepted", this->result() == QDialog::Accepted );
	return result;
}
