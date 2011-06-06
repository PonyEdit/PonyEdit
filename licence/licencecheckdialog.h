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
	void getTrialSucceeded(const QString& key);
	void getLicenceSucceeded(const QString& licence);
	void getLicenceFailed(const QString& error);
	void validateOnline();

protected:
	bool validateLicenceKey(const QString& key, bool trial);

private:
    Ui::LicenceCheckDialog *ui;
};

#endif // LICENCECHECKDIALOG_H
