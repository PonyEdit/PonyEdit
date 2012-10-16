#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QList>
#include "optionsdialogpage.h"
#include <QToolButton>
#include <QMap>

class FontOptionsWidget;

namespace Ui {
    class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
	enum Options { Editor, SshServers, FontsAndColors, Startup, Logging, NumOptions };

	explicit OptionsDialog(QWidget *parent = 0);
    ~OptionsDialog();

private slots:
	void buttonClicked(QAbstractButton *button);
	void saveOptions();
	void pageClicked();
	void pageClicked(QToolButton* page);

private:
	void addPage(QToolButton* button, OptionsDialogPage* page);

	Ui::OptionsDialog* ui;
	QList<OptionsDialogPage*> mPages;
	QMap<QToolButton*, OptionsDialogPage*> mPageMap;
};

#endif // OPTIONSDIALOG_H
