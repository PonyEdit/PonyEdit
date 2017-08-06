#ifndef OPTIONSDIALOGPAGE_H
#define OPTIONSDIALOGPAGE_H

#include <QWidget>

class OptionsDialogPage : public QWidget {
	Q_OBJECT
public:
	explicit OptionsDialogPage( QWidget *parent = 0 );

	virtual void apply() {}
};

#endif // OPTIONSDIALOGPAGE_H
