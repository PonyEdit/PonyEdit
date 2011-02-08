#ifndef STARTUPOPTIONSWIDGET_H
#define STARTUPOPTIONSWIDGET_H

#include <QWidget>
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

	void apply();

public slots:
	void SetFilesToggled(bool checked);
	void SetFilesToCurrent();

private:
	Ui::StartupOptionsWidget *ui;
};

#endif // STARTUPOPTIONSWIDGET_H
