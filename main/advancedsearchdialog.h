#ifndef ADVANCEDSEARCHDIALOG_H
#define ADVANCEDSEARCHDIALOG_H

HIDE_COMPILE_WARNINGS

#include <QDialog>
#include <QFile>

UNHIDE_COMPILE_WARNINGS

namespace Ui { class AdvancedSearchDialog; }
class BaseFile;

class AdvancedSearchDialog : public QDialog
{
    Q_OBJECT

public:
	enum Scope { CurrentFile, OpenFiles };

    explicit AdvancedSearchDialog(QWidget *parent = 0);
    ~AdvancedSearchDialog();

	AdvancedSearchDialog(AdvancedSearchDialog const&) = delete;
	AdvancedSearchDialog& operator=(AdvancedSearchDialog const&) = delete;

private slots:
	void search();
	void searchAndReplace();

private:
	QList<BaseFile*> getLocalHaystackFiles();

    Ui::AdvancedSearchDialog *ui;
};

#endif // ADVANCEDSEARCHDIALOG_H
