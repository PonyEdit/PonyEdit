#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QList>

namespace Ui {
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
	enum Options { Editor, SshServers, NumOptions };
	static const char* sOptionsStrings[];

	explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

private slots:
	void updateSelectedOption(int newOption);
	void buttonClicked(QAbstractButton *button);
	void saveOptions();

private:
    Ui::OptionsDialog *ui;

	void addSshServers();
};

#endif // OPTIONSDIALOG_H
