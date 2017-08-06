#ifndef EDITOROPTIONSWIDGET_H
#define EDITOROPTIONSWIDGET_H

HIDE_COMPILE_WARNINGS

#include <QWidget>

UNHIDE_COMPILE_WARNINGS

#include "optionsdialogpage.h"

namespace Ui { class EditorOptionsWidget; }

class EditorOptionsWidget : public OptionsDialogPage
{
    Q_OBJECT

public:
    explicit EditorOptionsWidget(QWidget *parent = 0);
    ~EditorOptionsWidget();

	EditorOptionsWidget(EditorOptionsWidget const&) = delete;
	EditorOptionsWidget& operator=(EditorOptionsWidget const&) = delete;
			
	virtual void apply();

private:
    Ui::EditorOptionsWidget *ui;
};

#endif // EDITOROPTIONSWIDGET_H
