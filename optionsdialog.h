#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
	enum Options { TextEditor, NumOptions };
	static const char* sOptionsStrings[];

	explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

private slots:
	void updateSelectedOption(int newOption);
	void buttonClicked(QAbstractButton *button);
	void saveOptions();

private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
