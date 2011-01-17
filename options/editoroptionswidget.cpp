#include "editoroptionswidget.h"
#include "ui_editoroptionswidget.h"
#include "options.h"

EditorOptionsWidget::EditorOptionsWidget(QWidget *parent) :
	OptionsDialogPage(parent),
    ui(new Ui::EditorOptionsWidget)
{
    ui->setupUi(this);

	ui->tabStopWidth->setText(QString::number(Options::TabStopWidth));
	ui->wordWrap->setChecked(Options::WordWrap);
}

EditorOptionsWidget::~EditorOptionsWidget()
{
    delete ui;
}

void EditorOptionsWidget::apply()
{
	bool ok;
	int tabStopWidth = ui->tabStopWidth->text().toInt(&ok);
	if (!ok)
		tabStopWidth = 80;

	Options::TabStopWidth = tabStopWidth;
	Options::WordWrap = ui->wordWrap->isChecked();
}
