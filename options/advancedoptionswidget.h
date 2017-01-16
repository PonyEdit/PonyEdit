#ifndef ADVANCEDOPTIONSWIDGET_H
#define ADVANCEDOPTIONSWIDGET_H

HIDE_COMPILE_WARNINGS

#include <QWidget>

UNHIDE_COMPILE_WARNINGS

#include "optionsdialogpage.h"

namespace Ui {
class AdvancedOptionsWidget;
}

class AdvancedOptionsWidget : public OptionsDialogPage
{
	Q_OBJECT
	
public:
	explicit AdvancedOptionsWidget(QWidget *parent = 0);
	~AdvancedOptionsWidget();

	AdvancedOptionsWidget(AdvancedOptionsWidget const&) = delete;
	AdvancedOptionsWidget& operator=(AdvancedOptionsWidget const&) = delete;
					
	void apply();

private slots:
	void setTrace();
	void setDebug();
	void setInfo();
	void setWarn();
	void setError();
	void setFatal();
	
private:
	Ui::AdvancedOptionsWidget *ui;
};

#endif // ADVANCEDOPTIONSWIDGET_H
