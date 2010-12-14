#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
    class PasswordDialog;
}

class PasswordDialog : public QDialog
{
    Q_OBJECT

public:
	static QString getPassword(QWidget* parent, const QString& prompt);

private:
    explicit PasswordDialog(QWidget *parent = 0);
    ~PasswordDialog();

    Ui::PasswordDialog *ui;
};

#endif // PASSWORDDIALOG_H
