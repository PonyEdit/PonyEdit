#include "passwordinput.h"
#include "ui_passwordinput.h"

PasswordInput::PasswordInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PasswordInput)
{
    ui->setupUi(this);
	ui->password->setFocus();
}

PasswordInput::~PasswordInput()
{
    delete ui;
}

QString PasswordInput::getEnteredPassword()
{
	return ui->password->text();
}

bool PasswordInput::getSavePassword()
{
	return ui->savePassword->isChecked();
}
