#ifndef UNSAVEDCHANGESDIALOG_H
#define UNSAVEDCHANGESDIALOG_H

HIDE_COMPILE_WARNINGS

#include <QAbstractButton>
#include <QItemSelection>
#include <QDialog>
#include <QList>

UNHIDE_COMPILE_WARNINGS

class BaseFile;
class QDialogButtonBox;
class OpenFileTreeView;

class UnsavedChangesDialog : public QDialog
{
    Q_OBJECT

public:
	explicit UnsavedChangesDialog(const QList<BaseFile*>& files, bool closeFilesOnDiscard = true);
    ~UnsavedChangesDialog();

	UnsavedChangesDialog(UnsavedChangesDialog const&) = delete;
	UnsavedChangesDialog& operator=(UnsavedChangesDialog const&) = delete;

private slots:
	void buttonClicked(QAbstractButton* button);
	void selectionChanged(QItemSelection before, QItemSelection after);
	void fileStateChanged();
	void fileClosed(BaseFile* file);

private:
	QDialogButtonBox* mButtonBox;
	OpenFileTreeView* mTreeView;
	bool mCloseFilesOnDiscard;
};

#endif // UNSAVEDCHANGESDIALOG_H
