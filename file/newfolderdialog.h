#ifndef NEWFOLDERDIALOG_H
#define NEWFOLDERDIALOG_H

#include <QDialog>
#include "file/location.h"

namespace Ui {
class NewFolderDialog;
}

class NewFolderDialog : public QDialog {
	Q_OBJECT

	public:
		explicit NewFolderDialog( QWidget *parent, const Location& parentLocation );
		~NewFolderDialog();

		virtual void accept();

	private slots:
		void createSuccess( QVariantMap result );
		void createFailure( QString error, int flags );

	private:
		void attempt( bool sudo );

		Ui::NewFolderDialog *ui;
		Location mParentLocation;
};

#endif  // NEWFOLDERDIALOG_H
