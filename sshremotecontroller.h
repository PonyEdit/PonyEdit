#ifndef SSHREMOTECONTROLLER_H
#define SSHREMOTECONTROLLER_H

#include <QString>
#include <QByteArray>
#include "sshconnection.h"
#include "remotefile.h"

#define SSH_SLAVE_FILE "slave.py"

class SshRemoteController
{
public:
	SshRemoteController();
	void attach(SshConnection* connection);

	QByteArray openFile(const char* filename);

private:
	SshConnection* mSsh;
	QString mHomeDirectory;

	static bool sSlaveLoaded;
	static QByteArray sSlaveScript;
	static QByteArray sSlaveMd5;
};

#endif // SSHREMOTECONTROLLER_H
