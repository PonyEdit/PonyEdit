#ifndef GOTOLINEDIALOG_H
#define GOTOLINEDIALOG_H

#include <QDialog>

namespace Ui {
class GotoLineDialog;
}

class GotoLineDialog : public QDialog {
	Q_OBJECT

	public:
		explicit GotoLineDialog( QWidget *parent = nullptr );
		~GotoLineDialog();

		int lineNumber();

	public slots:
		void accept();

	private:
		Ui::GotoLineDialog *ui;
		int mLineNumber;
};

#endif  // GOTOLINEDIALOG_H
