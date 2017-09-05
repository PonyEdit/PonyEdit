#ifndef EDITOROPTIONSWIDGET_H
#define EDITOROPTIONSWIDGET_H

#include <QWidget>
#include "optionsdialogpage.h"

namespace Ui { class EditorOptionsWidget; }

class EditorOptionsWidget : public OptionsDialogPage {
	Q_OBJECT

	public:
		explicit EditorOptionsWidget( QWidget *parent = 0 );
		~EditorOptionsWidget();

		virtual void apply();

	private:
		Ui::EditorOptionsWidget *ui;
};

#endif  // EDITOROPTIONSWIDGET_H
