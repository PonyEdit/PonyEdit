#ifndef HOSTKEYDLG_H
#define HOSTKEYDLG_H

#include <QDialog>
#include "threadcrossingdialog.h"

namespace Ui {
class HostKeyDlg;
}

class HostKeyDlg : public ThreadCrossingDialog {
	Q_OBJECT

	public:
		explicit HostKeyDlg( QWidget *parent = 0 );
		~HostKeyDlg();

		virtual void setOptions( const QVariantMap &options );

	private:
		Ui::HostKeyDlg *ui;
};

#endif  // HOSTKEYDLG_H
