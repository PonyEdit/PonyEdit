#ifndef SSHREMOTECONTROLLER_H
#define SSHREMOTECONTROLLER_H

#include <QString>
#include <QObject>
#include <QByteArray>

class SshControllerThread;
class SshRequest;
class SshHost;

#define	KEEPALIVE_TIMEOUT 60000	/* 60 seconds */

class SshRemoteController : public QObject
{
	Q_OBJECT

public:
	enum Status { NotConnected, Connecting, WaitingForPassword, Negotiating, UploadingSlave, StartingSlave, PushingBuffers, Connected, Error };
	static const char* sStatusStrings[];

	enum ScriptType { AutoDetect, Python, Perl, NumScriptTypes };
	static const char* sScriptTypeLabels[];

	SshRemoteController(SshHost* host);
	~SshRemoteController();

	void abortConnection();
	void sendRequest(SshRequest* request);
	const QString& getHomeDirectory() const;

	int getLastStatusChange() const;
	Status getStatus() const;
	static const char* getStatusString(Status s) { return sStatusStrings[s]; }
	const QString& getError() const;

	void emitStateChanged() { emit stateChanged(); }

signals:
	void stateChanged();

private:
	SshControllerThread* mThread;
};

#endif // SSHREMOTECONTROLLER_H
