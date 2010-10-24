#ifndef SSHREMOTECONTROLLER_H
#define SSHREMOTECONTROLLER_H

#include <QString>
#include <QByteArray>
#include <QThread>
#include <QMutex>
#include "sshconnection.h"
#include "sshrequest.h"
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

	void splitThread();
	void sendRequest(SshRequest* request);

	inline const QString& getHomeDirectory() const { return mHomeDirectory; }

private:
	class ControllerThread : public QThread
	{
	public:
		ControllerThread() {}
		void run();
		SshConnection* mSsh;

		QList<SshRequest*> mRequestQueue;
		QMutex mRequestQueueLock;
	};

	SshConnection* mSsh;
	QString mHomeDirectory;
	ControllerThread mThread;

	static bool sSlaveLoaded;
	static QByteArray sSlaveScript;
	static QByteArray sSlaveMd5;
};

#endif // SSHREMOTECONTROLLER_H
