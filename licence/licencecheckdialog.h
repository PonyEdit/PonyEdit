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
	void getLicenceSucceeded(const QString& licence);
	void getLicenceFailed(const QString& error);
	void validateOnline();
	void validateOffline();

protected:
	bool validateLicenceKey(const QString& key);

private:
    Ui::LicenceCheckDialog *ui;
};

#endif // LICENCECHECKDIALOG_H
