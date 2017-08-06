#ifndef SHUTDOWNPROMPT_H
#define SHUTDOWNPROMPT_H

#include <QDialog>

namespace Ui {
	class ShutdownPrompt;
}

class ShutdownPrompt : public QDialog {
	Q_OBJECT

public:
	explicit ShutdownPrompt( QWidget *parent = 0 );
	~ShutdownPrompt();

public slots:
	void remember();
	void dontRemember();

private:
	Ui::ShutdownPrompt *ui;
};

#endif // SHUTDOWNPROMPT_H
