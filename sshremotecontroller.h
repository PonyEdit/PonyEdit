#ifndef SSHREMOTECONTROLLER_H
#define SSHREMOTECONTROLLER_H

#include <QString>
#include <QByteArray>
#include "sshconnection.h"
#include "sshrequest.h"

#define SSH_SLAVE_FILE "slave.py"

class SshControllerThread;
class SshRequest;

class SshRemoteController
{
public:
	enum Status { NotConnected, Connecting, WaitingForPassword, Negotiating, UploadingSlave, StartingSlave, Connected, Error };
	static const char* sStatusStrings[];

	SshRemoteController(SshHost* host);
	~SshRemoteController();

	void abortConnection();
	void sendRequest(SshRequest* request);
	const QString& getHomeDirectory() const;

	int getLastStatusChange() const;
	Status getStatus() const;
	static const char* getStatusString(Status s) { return sStatusStrings[s]; }
	const QString& getError() const;

private:
	SshControllerThread* mThread;
};

#endif // SSHREMOTECONTROLLER_H
