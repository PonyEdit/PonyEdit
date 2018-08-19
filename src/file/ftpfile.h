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
		void sftpReadSuccess( const QVariantMap &results );
		void sftpReadFailure( const QString &error, int flags );
		void sftpReadProgress( int progress );

		void sftpWriteSuccess( const QVariantMap &results );
		void sftpWriteFailure( const QString &error, int flags );
		void sftpWriteProgress( int progress );

	private:
		SshHost *mHost;
};

#endif  // FTPFILE_H
