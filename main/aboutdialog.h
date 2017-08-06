#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

HIDE_COMPILE_WARNINGS

#include <QDialog>

UNHIDE_COMPILE_WARNINGS

namespace Ui {
    class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

	AboutDialog(AboutDialog const&) = delete;
	AboutDialog& operator=(AboutDialog const&) = delete;

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
