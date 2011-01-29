#ifndef PONYEDIT_H
#define PONYEDIT_H

#include <QApplication>

class PonyEdit : public QApplication
{
public:
	PonyEdit(int argc, char** argv);

public slots:
	bool event(QEvent *e);
};

#endif // PONYEDIT_H
