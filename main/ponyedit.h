#ifndef PONYEDIT_H
#define PONYEDIT_H

#include <QApplication>
#include <QSharedMemory>
#include <QLocalServer>

#include "mainwindow.h"

class PonyEdit : public QApplication
{
	Q_OBJECT

public:
	PonyEdit(int argc, char** argv);
	~PonyEdit();

	bool isRunning();
	bool sendMessage(const QString &message);

public slots:
	bool event(QEvent *e);
	void receiveMessage();

signals:
	void messageAvailable(QString message);

private:
	bool mIsRunning;
	QString mKey;
	QSharedMemory mMemoryLock;
	QLocalServer* mLocalServer;

	static const int mTimeout = 1000;
};

#endif // PONYEDIT_H
