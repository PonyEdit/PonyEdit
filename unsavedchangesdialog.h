#ifndef UNSAVEDCHANGESDIALOG_H
#define UNSAVEDCHANGESDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include <QList>

namespace Ui { class UnsavedChangesDialog; }

class BaseFile;
class UnsavedChangesDialog : public QDialog
{
    Q_OBJECT

public:
	explicit UnsavedChangesDialog(QList<BaseFile*> files);
    ~UnsavedChangesDialog();

private slots:
	void buttonClicked(QAbstractButton* button);
	void selectionChanged();

private:
    Ui::UnsavedChangesDialog *ui;
	QList<BaseFile*> mFiles;
};

#endif // UNSAVEDCHANGESDIALOG_H
