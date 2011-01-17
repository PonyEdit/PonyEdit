#include "editoroptionswidget.h"
#include "ui_editoroptionswidget.h"
#include "options.h"

EditorOptionsWidget::EditorOptionsWidget(QWidget *parent) :
	OptionsDialogPage(parent),
    ui(new Ui::EditorOptionsWidget)
{
    ui->setupUi(this);

	ui->tabStopWidth->setValue(Options::TabStopWidth);
	ui->wordWrap->setChecked(Options::WordWrap);
	ui->keepIndent->setChecked(Options::IndentMode == Options::KeepIndentOnNextLine);
}

EditorOptionsWidget::~EditorOptionsWidget()
{
    delete ui;
}

void EditorOptionsWidget::apply()
{
	Options::TabStopWidth = ui->tabStopWidth->value();
	Options::WordWrap = ui->wordWrap->isChecked();
	Options::IndentMode = ui->keepIndent->isChecked() ? Options::KeepIndentOnNextLine : Options::NoAutoIndent;
}
