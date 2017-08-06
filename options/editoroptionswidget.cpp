#include "editoroptionswidget.h"
#include "options.h"
#include "ui_editoroptionswidget.h"

EditorOptionsWidget::EditorOptionsWidget( QWidget *parent )
    : OptionsDialogPage( parent ), ui( new Ui::EditorOptionsWidget ) {
	ui->setupUi( this );

	ui->tabStopWidth->setValue( Options::TabStopWidth );
	ui->wordWrap->setChecked( Options::WordWrap );
	ui->keepIndent->setChecked( Options::IndentMode == Options::KeepIndentOnNextLine );
	ui->indentSpaces->setChecked( Options::IndentSpaces );
	ui->stripSpaces->setChecked( Options::StripSpaces );
}

EditorOptionsWidget::~EditorOptionsWidget() {
	delete ui;
}

void EditorOptionsWidget::apply() {
	Options::TabStopWidth = ui->tabStopWidth->value();
	Options::WordWrap     = ui->wordWrap->isChecked();
	Options::IndentMode   = ui->keepIndent->isChecked() ? Options::KeepIndentOnNextLine : Options::NoAutoIndent;
	Options::IndentSpaces = ui->indentSpaces->isChecked();
	Options::StripSpaces  = ui->stripSpaces->isChecked();
}
