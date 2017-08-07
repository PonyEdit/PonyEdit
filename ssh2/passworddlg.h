#ifndef PASSWORDDLG_H
#define PASSWORDDLG_H

#include "threadcrossingdialog.h"

namespace Ui {
class PasswordDlg;
}

class PasswordDlg : public ThreadCrossingDialog
{
Q_OBJECT

public:
explicit PasswordDlg( QWidget *parent = 0 );
~PasswordDlg();

virtual void setOptions( const QVariantMap &options );
virtual QVariantMap getResult();

private:
Ui::PasswordDlg *ui;
};

#endif	// PASSWORDDLG_H
