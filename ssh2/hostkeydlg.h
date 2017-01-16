#ifndef HOSTKEYDLG_H
#define HOSTKEYDLG_H

HIDE_COMPILE_WARNINGS

#include <QDialog>

UNHIDE_COMPILE_WARNINGS

#include "threadcrossingdialog.h"

namespace Ui {
    class HostKeyDlg;
}

class HostKeyDlg : public ThreadCrossingDialog
{
    Q_OBJECT

public:
    explicit HostKeyDlg(QWidget *parent = 0);
    ~HostKeyDlg();

	HostKeyDlg(HostKeyDlg const&) = delete;
	HostKeyDlg& operator=(HostKeyDlg const&) = delete;
	
	virtual void setOptions(const QVariantMap &options);

private:
    Ui::HostKeyDlg *ui;
};

#endif // HOSTKEYDLG_H
