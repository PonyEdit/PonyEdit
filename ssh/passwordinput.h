#ifndef PASSWORDINPUT_H
#define PASSWORDINPUT_H

#include <QWidget>

namespace Ui {
    class PasswordInput;
}

class PasswordInput : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordInput(QWidget *parent = 0);
    ~PasswordInput();

	QString getEnteredPassword();
	bool getSavePassword();

private:
    Ui::PasswordInput *ui;
};

#endif // PASSWORDINPUT_H
