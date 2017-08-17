#ifndef FTPFILE_H
#define FTPFILE_H

#include "basefile.h"
#include "location.h"

class SshHost;
class FtpFile : public BaseFile {
	Q_OBJECT

	public:
		FtpFile( const Location &location );

		BaseFile *newFile( const QString &content );
		void open();
		void save();
		void close();
		void refresh();
		bool canClose();

	private slots:
		void sftpReadSuccess( QVariantMap results );
		void sftpReadFailure( QString error, int flags );
		void sftpReadProgress( int progress );

		void sftpWriteSuccess( QVariantMap results );
		void sftpWriteFailure( QString error, int flags );
		void sftpWriteProgress( int progress );

	private:
		SshHost *mHost;
};

#endif  // FTPFILE_H
