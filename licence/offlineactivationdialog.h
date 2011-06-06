#ifndef OFFLINEACTIVATIONDIALOG_H
#define OFFLINEACTIVATIONDIALOG_H

#include <QDialog>
#include "licence.h"

namespace Ui {
    class OfflineActivationDialog;
}

class OfflineActivationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OfflineActivationDialog(QWidget *parent = 0);
    ~OfflineActivationDialog();

public slots:
	void validate();
	void accept();

	inline QString getLicenceKey() { return mLicence.getKey(); }

private:
	Ui::OfflineActivationDialog *ui;
	Licence mLicence;
};

#endif // OFFLINEACTIVATIONDIALOG_H
