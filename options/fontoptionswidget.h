#ifndef FONTOPTIONSWIDGET_H
#define FONTOPTIONSWIDGET_H

HIDE_COMPILE_WARNINGS

#include <QWidget>
#include <QFontDatabase>

UNHIDE_COMPILE_WARNINGS

#include "optionsdialogpage.h"

namespace Ui {
    class fontoptionswidget;
}

class FontOptionsWidget : public OptionsDialogPage
{
    Q_OBJECT

public:
	explicit FontOptionsWidget(QWidget *parent = 0);
	~FontOptionsWidget();

	FontOptionsWidget(FontOptionsWidget const&) = delete;
	FontOptionsWidget& operator=(FontOptionsWidget const&) = delete;
		
	void apply();

private:
    Ui::fontoptionswidget *ui;
	QFontDatabase mFontDatabase;
};

#endif // FONTOPTIONSWIDGET_H
