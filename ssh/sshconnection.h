#ifndef SSHREMOTECONTROLLER_H
#define SSHREMOTECONTROLLER_H

#include <QString>
#include <QByteArray>
#include <QDialogButtonBox>
#include <QMutex>

#include "remoteconnection.h"
#include "rawsshconnection.h"

class SlaveRequest;
class SshHost;
class PasswordInput;
class RawSshChannelHandle;

class SshConnection : public RemoteConnection
{
public:
	SshConnection(SshHost* host);
	~SshConnection();

	inline RawSshConnection* getRawConnection() const { return mRawConnection; }

	virtual QString getName();
	virtual RawChannelHandle* createRawSlaveChannel();

	virtual void sendLine(RawChannelHandle* handle, const QByteArray& data);
	virtual QByteArray readLine(RawChannelHandle* handle);

protected:
	virtual bool threadConnect();
	virtual RemoteChannel* threadOpenPrimaryChannel();

	static void hostkeyWarnDialog(ConnectionStatusWidget* widget, RemoteConnection* connection, QWidget* target, QVariant param);
	static bool hostkeyWarnCallback(ConnectionStatusWidget* widget, RemoteConnection* connection, QDialogButtonBox::ButtonRole buttonRole, QVariant param);

	static void passwordInputDialog(ConnectionStatusWidget* widget, RemoteConnection* connection, QWidget* target, QVariant param);
	static bool passwordInputCallback(ConnectionStatusWidget* widget, RemoteConnection* connection, QDialogButtonBox::ButtonRole buttonRole, QVariant param);

private:
	void loadSlaveScript();

private:
	SshHost* mHost;
	RawSshConnection* mRawConnection;

	static QMutex sSlaveScriptMutex;	//	Mutex used to ensure two slave threads don't try to init the slaveScript simultaneously
	static bool sSlaveLoaded;
	static QByteArray sSlaveScript;
	static QByteArray sSlaveMd5;

	PasswordInput* mPasswordInput;
};

#endif // SSHREMOTECONTROLLER_H
