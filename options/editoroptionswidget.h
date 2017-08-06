#ifndef EDITOROPTIONSWIDGET_H
#define EDITOROPTIONSWIDGET_H

#include "optionsdialogpage.h"
#include <QWidget>

namespace Ui {
	class EditorOptionsWidget;
}

class EditorOptionsWidget : public OptionsDialogPage {
	Q_OBJECT

public:
	explicit EditorOptionsWidget( QWidget *parent = 0 );
	~EditorOptionsWidget();

	virtual void apply();

private:
	Ui::EditorOptionsWidget *ui;
};

#endif // EDITOROPTIONSWIDGET_H
