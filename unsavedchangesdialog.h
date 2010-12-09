#ifndef UNSAVEDCHANGESDIALOG_H
#define UNSAVEDCHANGESDIALOG_H

#include <QAbstractButton>
#include <QItemSelection>
#include <QDialog>
#include <QList>

class BaseFile;
class QDialogButtonBox;
class OpenFileTreeView;

class UnsavedChangesDialog : public QDialog
{
    Q_OBJECT

public:
	explicit UnsavedChangesDialog(const QList<BaseFile*>& files);
    ~UnsavedChangesDialog();

private slots:
	void buttonClicked(QAbstractButton* button);
	void selectionChanged(QItemSelection before, QItemSelection after);
	void fileStateChanged();

private:
	QDialogButtonBox* mButtonBox;
	OpenFileTreeView* mTreeView;
};

#endif // UNSAVEDCHANGESDIALOG_H
