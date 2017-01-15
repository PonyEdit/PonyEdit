#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

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
