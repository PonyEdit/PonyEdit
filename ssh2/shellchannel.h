#ifndef SHELLCHANNEL_H
#define SHELLCHANNEL_H

#include "sshchannel.h"
#define SSH_SHELL_BUFFER_SIZE 4096

struct _LIBSSH2_CHANNEL;
typedef _LIBSSH2_CHANNEL LIBSSH2_CHANNEL;

class ShellChannel : public SshChannel {
	Q_OBJECT

	public:
		ShellChannel( SshHost* host, bool machineReadable = true, const QByteArray& ptyType = "vanilla" );

// Pty types = vanilla, vt102, ansi or xterm.

		bool update();
		virtual int getConnectionScore();
		virtual QString getConnectionDescription();
		virtual Type getType() {
			return Shell;
		}

	signals:
		void connected();
		void error( QString message );

	protected:
		struct ReadReply { bool readAgain; QByteArray data; };
		enum SendResponse { SendAgain, SendFail, SendSucceed };
		enum InternalStatus { _OpenSession = 30, _RequestPty = 31, _StartShell = 32, _SendInit = 33, _WaitForInitReply = 34 };

		virtual void shellReady();
		bool handleOpening();

		SendResponse sendData( const QByteArray& data );
		ReadReply readUntilPrompt();
		ReadReply readUntil( const QByteArray& marker );

		bool mainUpdate();

		LIBSSH2_CHANNEL* mHandle;

		QByteArray mReadBuffer;
		char mScratchBuffer[SSH_SHELL_BUFFER_SIZE];

	private:
		void setInternalStatus( InternalStatus newStatus );
		InternalStatus mInternalStatus;

		bool mMachineReadable;
		QByteArray mPtyType;
};

#endif  // SHELLCHANNEL_H
