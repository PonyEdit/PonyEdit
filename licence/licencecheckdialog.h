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
    explicit LicenceCheckDialog(QWidget *parent = 0);
    ~LicenceCheckDialog();

private:
    Ui::LicenceCheckDialog *ui;
};

#endif // LICENCECHECKDIALOG_H
