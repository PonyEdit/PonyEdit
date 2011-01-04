#ifndef ADVANCEDSEARCHDIALOG_H
#define ADVANCEDSEARCHDIALOG_H

#include <QDialog>

namespace Ui {
    class AdvancedSearchDialog;
}

class AdvancedSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdvancedSearchDialog(QWidget *parent = 0);
    ~AdvancedSearchDialog();

signals:
	void find(const QString& text, bool backwards, bool caseSensitive, bool useRegexp);
	void globalFind(const QString& text, const QString& filePattern, bool backwards, bool caseSensitive, bool useRegexp);

	void replace(const QString& findText, const QString& replaceText, bool caseSensitive, bool useRegexp, bool all);
	void globalReplace(const QString& text, const QString& replaceText, const QString& filePattern, bool caseSensitive, bool useRegexp, bool all);

private slots:
	void findNext();
	void findPrevious();
	void replaceCurrent();
	void replaceCurrentAndFind();
	void replaceAll();

private:
    Ui::AdvancedSearchDialog *ui;
};

#endif // ADVANCEDSEARCHDIALOG_H
