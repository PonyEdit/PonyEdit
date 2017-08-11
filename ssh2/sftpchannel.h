#ifndef SFTPCHANNEL_H
#define SFTPCHANNEL_H

#include <QVariantMap>
#include "sshchannel.h"

struct _LIBSSH2_SFTP;
typedef _LIBSSH2_SFTP LIBSSH2_SFTP;

struct _LIBSSH2_SFTP_HANDLE;
typedef _LIBSSH2_SFTP_HANDLE LIBSSH2_SFTP_HANDLE;

class SFTPRequest;
class SFTPChannel : public SshChannel {
	public:
		SFTPChannel( SshHost* host );

		virtual bool update();
		virtual Type getType() {
			return SshChannel::Sftp;
		}

	protected:
		void criticalError( const QString& error );
		bool handleOpening();
		bool mainUpdate();
		bool updateLs();
		bool updateMkDir();
		bool updateReadFile();
		bool updateWriteFile();

	private:
		enum RequestState { Beginning, Sizing, Reading, Writing, Finishing };

		LIBSSH2_SFTP* mHandle;
		LIBSSH2_SFTP_HANDLE* mOperationHandle;
		SFTPRequest* mCurrentRequest;
		RequestState mRequestState;

		QVariantMap mResult;
		int mOperationSize;
		int mOperationCursor;
};

#endif  // SFTPCHANNEL_H
