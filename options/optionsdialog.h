#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QList>
#include "optionsdialogpage.h"

class FontOptionsWidget;

namespace Ui {
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
	enum Options { Editor, SshServers, FontsAndColors, NumOptions };
	static QString sOptionsStrings[];

	explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

private slots:
	void updateSelectedOption(int newOption);
	void buttonClicked(QAbstractButton *button);
	void saveOptions();

private:
	void addPage(OptionsDialogPage* page);

	Ui::OptionsDialog* ui;
	QList<OptionsDialogPage*> mPages;
};

#endif // OPTIONSDIALOG_H
