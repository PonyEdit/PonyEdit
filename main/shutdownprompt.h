#ifndef SHUTDOWNPROMPT_H
#define SHUTDOWNPROMPT_H

HIDE_COMPILE_WARNINGS

#include <QDialog>

UNHIDE_COMPILE_WARNINGS

namespace Ui {
    class ShutdownPrompt;
}

class ShutdownPrompt : public QDialog
{
    Q_OBJECT

public:
    explicit ShutdownPrompt(QWidget *parent = 0);
    ~ShutdownPrompt();

	ShutdownPrompt(ShutdownPrompt const&) = delete;
	ShutdownPrompt& operator=(ShutdownPrompt const&) = delete;
		
public slots:
	void remember();
	void dontRemember();

private:
    Ui::ShutdownPrompt *ui;
};

#endif // SHUTDOWNPROMPT_H
