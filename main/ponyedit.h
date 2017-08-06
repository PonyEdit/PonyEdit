#ifndef PONYEDIT_H
#define PONYEDIT_H

#include <QApplication>
#include <QLocalServer>
#include <QSharedMemory>

#include "mainwindow.h"
#include "ssh2/dialogrethreader.h"
#include "tools.h"

class PonyEdit : public QApplication {
	Q_OBJECT

public:
	PonyEdit( int argc, char **argv );
	~PonyEdit();

	bool isRunning();
	bool sendMessage( const QString &message );

	const QStringList &getPositionalArguments() {
		return mPositionalArguments;
	}
	static inline bool isApplicationExiting() {
		return sApplicationExiting;
	}

public slots:
	bool event( QEvent *e );
	void receiveMessage();
	bool notify( QObject *, QEvent * );
	void loadStartupFiles() {
		Tools::loadStartupFiles();
	}

signals:
	void messageAvailable( QString message );

private:
	bool              mIsRunning;
	QString           mKey;
	QSharedMemory     mMemoryLock;
	QLocalServer *    mLocalServer;
	DialogRethreader *mDialogRethreader;

	QStringList mPositionalArguments;

	static const int mTimeout = 1000;
	static bool      sApplicationExiting;
};

#endif // PONYEDIT_H
