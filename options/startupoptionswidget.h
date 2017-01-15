#ifndef STARTUPOPTIONSWIDGET_H
#define STARTUPOPTIONSWIDGET_H

HIDE_COMPILE_WARNINGS

#include <QWidget>

UNHIDE_COMPILE_WARNINGS

#include "optionsdialogpage.h"

namespace Ui {
	class StartupOptionsWidget;
}

class StartupOptionsWidget : public OptionsDialogPage
{
    Q_OBJECT

public:
	explicit StartupOptionsWidget(QWidget *parent = 0);
	~StartupOptionsWidget();

	StartupOptionsWidget(StartupOptionsWidget const&) = delete;
	StartupOptionsWidget& operator=(StartupOptionsWidget const&) = delete;
				
	void apply();

public slots:
	void SetFilesToggled(bool checked);
	void SetFilesToCurrent();

private:
	Ui::StartupOptionsWidget *ui;
};

#endif // STARTUPOPTIONSWIDGET_H
