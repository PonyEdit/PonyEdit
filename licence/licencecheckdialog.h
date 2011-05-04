#ifndef LICENCECHECKDIALOG_H
#define LICENCECHECKDIALOG_H

#include <QDialog>

namespace Ui {
    class LicenceCheckDialog;
}

class LicenceCheckDialog : public QDialog
{
    Q_OBJECT

public:
	explicit LicenceCheckDialog(QWidget* parent, bool expired);
    ~LicenceCheckDialog();

public slots:
	void getTrial();
	void saveTrial(const QString& key);
	void getTrialFailed(const QString& error);

private:
    Ui::LicenceCheckDialog *ui;
};

#endif // LICENCECHECKDIALOG_H
