#ifndef GOTOLINEDIALOG_H
#define GOTOLINEDIALOG_H

HIDE_COMPILE_WARNINGS

#include <QDialog>

UNHIDE_COMPILE_WARNINGS

namespace Ui {
    class GotoLineDialog;
}

class GotoLineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GotoLineDialog(QWidget *parent = 0);
    ~GotoLineDialog();

	GotoLineDialog(GotoLineDialog const&) = delete;
	GotoLineDialog& operator=(GotoLineDialog const&) = delete;

	int lineNumber();

public slots:
	void accept();

private:
    Ui::GotoLineDialog *ui;
	int mLineNumber;
};

#endif // GOTOLINEDIALOG_H
