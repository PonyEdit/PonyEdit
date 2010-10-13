#ifndef SSHREMOTECONTROLLER_H
#define SSHREMOTECONTROLLER_H

#include <QByteArray>
#include "sshconnection.h"

#define SSH_SLAVE_FILE "slave.py"

class SshRemoteController
{
public:
	SshRemoteController();
	void attach(SshConnection* connection);

private:
	SshConnection* mSsh;

	static bool sSlaveLoaded;
	static QByteArray sSlaveScript;
	static QByteArray sSlaveMd5;
};

#endif // SSHREMOTECONTROLLER_H
