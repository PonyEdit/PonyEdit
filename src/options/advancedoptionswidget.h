#ifndef ADVANCEDOPTIONSWIDGET_H
#define ADVANCEDOPTIONSWIDGET_H

#include <QWidget>
#include "optionsdialogpage.h"

namespace Ui {
class AdvancedOptionsWidget;
}

class AdvancedOptionsWidget : public OptionsDialogPage {
	Q_OBJECT

	public:
		explicit AdvancedOptionsWidget( QWidget *parent = nullptr );
		~AdvancedOptionsWidget();

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

#endif  // ADVANCEDOPTIONSWIDGET_H
