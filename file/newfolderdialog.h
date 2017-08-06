#ifndef NEWFOLDERDIALOG_H
#define NEWFOLDERDIALOG_H

HIDE_COMPILE_WARNINGS

#include <QDialog>

UNHIDE_COMPILE_WARNINGS

#include "file/location.h"

namespace Ui {
    class NewFolderDialog;
}

class NewFolderDialog : public QDialog
{
    Q_OBJECT

public:
	explicit NewFolderDialog(QWidget *parent, const Location& parentLocation);
    ~NewFolderDialog();

	NewFolderDialog(NewFolderDialog const&) = delete;
	NewFolderDialog& operator=(NewFolderDialog const&) = delete;
		
	virtual void accept();

private slots:
	void createSuccess(QVariantMap result);
	void createFailure(QString error, int flags);

private:
	void attempt(bool sudo);

    Ui::NewFolderDialog *ui;
	Location mParentLocation;
};

#endif // NEWFOLDERDIALOG_H
