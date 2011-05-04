#ifndef LICENCECHECKDIALOG_H
#define LICENCECHECKDIALOG_H

#include <QDialog>

namespace Ui {
    class LicenceCheckDialog;
}

class Licence;
class LicenceCheckDialog : public QDialog
{
    Q_OBJECT

public:
	explicit LicenceCheckDialog(QWidget* parent, Licence* currentLicence);
    ~LicenceCheckDialog();

public slots:
	void getTrial();
	void saveTrial(const QString& key);
	void getTrialFailed(const QString& error);

private:
    Ui::LicenceCheckDialog *ui;
};

#endif // LICENCECHECKDIALOG_H
