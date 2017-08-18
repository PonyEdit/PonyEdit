#ifndef ADVANCEDSEARCHDIALOG_H
#define ADVANCEDSEARCHDIALOG_H

#include <QDialog>
#include <QFile>

namespace Ui { class AdvancedSearchDialog; }
class BaseFile;

class AdvancedSearchDialog : public QDialog {
	Q_OBJECT

	public:
		enum Scope { CurrentFile, OpenFiles };

		explicit AdvancedSearchDialog( QWidget *parent = 0 );
		~AdvancedSearchDialog();

	private slots:
		void search();
		void searchAndReplace();

	private:
		QList< BaseFile * > getLocalHaystackFiles();

		Ui::AdvancedSearchDialog *ui;
};

#endif  // ADVANCEDSEARCHDIALOG_H
