#ifndef OPTIONSDIALOGPAGE_H
#define OPTIONSDIALOGPAGE_H

HIDE_COMPILE_WARNINGS

#include <QWidget>

UNHIDE_COMPILE_WARNINGS

class OptionsDialogPage : public QWidget
{
    Q_OBJECT
public:
    explicit OptionsDialogPage(QWidget *parent = 0);

	virtual void apply() {}
};

#endif // OPTIONSDIALOGPAGE_H
