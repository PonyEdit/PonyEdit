#ifndef SSHREMOTECONTROLLER_H
#define SSHREMOTECONTROLLER_H

#include <QString>
#include <QByteArray>
#include <QThread>
#include <QMutex>
#include "sshconnection.h"
#include "remotefile.h"

#define SSH_SLAVE_FILE "slave.py"

struct Push
{
	int position;
	int remove;
	QString add;
	bool save;
};

class SshRemoteController
{
public:
	SshRemoteController();
	void attach(SshConnection* connection);

	QByteArray openFile(const char* filename);

	void splitThread();
	void push(Push p);

private:
	class ControllerThread : public QThread
	{
	public:
		ControllerThread() {}
		void run();
		SshConnection* mSsh;
		QMutex mQueueLock;
		QList<Push> mQueue;
	};

	SshConnection* mSsh;
	QString mHomeDirectory;
	ControllerThread mThread;

	static bool sSlaveLoaded;
	static QByteArray sSlaveScript;
	static QByteArray sSlaveMd5;
};

#endif // SSHREMOTECONTROLLER_H
