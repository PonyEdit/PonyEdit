#ifndef FONTOPTIONSWIDGET_H
#define FONTOPTIONSWIDGET_H

#include <QFontDatabase>
#include <QWidget>
#include "optionsdialogpage.h"

namespace Ui {
class fontoptionswidget;
}

class FontOptionsWidget : public OptionsDialogPage
{
Q_OBJECT

public:
explicit FontOptionsWidget( QWidget *parent = 0 );
~FontOptionsWidget();

void apply();

private:
Ui::fontoptionswidget *ui;
QFontDatabase mFontDatabase;
};

#endif	// FONTOPTIONSWIDGET_H
