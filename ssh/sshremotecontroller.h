#ifndef SSHREMOTECONTROLLER_H
#define SSHREMOTECONTROLLER_H

#include <QString>
#include <QByteArray>
#include <QDialogButtonBox>

#include "remoteconnection.h"

class SshControllerThread;
class SshRequest;
class SshHost;
class PasswordInput;

#define	KEEPALIVE_TIMEOUT 60000	/* 60 seconds */

class SshRemoteController : public RemoteConnection
{
public:
	SshRemoteController(SshHost* host);
	~SshRemoteController();

	QString getName();

	void abortConnection();
	void sendRequest(SshRequest* request);
	const QString& getHomeDirectory() const;

	static void hostkeyWarnDialog(ConnectionStatusWidget* widget, RemoteConnection* connection, QWidget* target);
	static bool hostkeyWarnCallback(ConnectionStatusWidget* widget, RemoteConnection* connection, QDialogButtonBox::ButtonRole buttonRole);

	static void passwordInputDialog(ConnectionStatusWidget* widget, RemoteConnection* connection, QWidget* target);
	static bool passwordInputCallback(ConnectionStatusWidget* widget, RemoteConnection* connection, QDialogButtonBox::ButtonRole buttonRole);

	bool mKeyPassphraseInput;
private:
	SshControllerThread* mThread;
	PasswordInput* mPasswordInput;
	bool mNewHostKey;
};

#endif // SSHREMOTECONTROLLER_H
