#ifndef UNSAVEDCHANGESDIALOG_H
#define UNSAVEDCHANGESDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include <QItemSelection>
#include <QList>

class BaseFile;
class QDialogButtonBox;
class OpenFileTreeView;

class UnsavedChangesDialog : public QDialog {
	Q_OBJECT

	public:
		explicit UnsavedChangesDialog( const QList< BaseFile * > &files, bool closeFilesOnDiscard = true );
		~UnsavedChangesDialog();

	private slots:
		void buttonClicked( QAbstractButton *button );
		void selectionChanged( const QItemSelection &before, const QItemSelection &after );
		void fileStateChanged();
		void fileClosed( BaseFile *file );

	private:
		QDialogButtonBox *mButtonBox;
		OpenFileTreeView *mTreeView;
		bool mCloseFilesOnDiscard;
};

#endif  // UNSAVEDCHANGESDIALOG_H
