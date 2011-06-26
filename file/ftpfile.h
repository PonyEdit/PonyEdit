#ifndef FTPFILE_H
#define FTPFILE_H

#include "basefile.h"
#include "location.h"

class RemoteConnection;
class FTPChannel;

class FtpFile : public BaseFile
{
	Q_OBJECT

public:
	FtpFile(const Location& location);

	BaseFile* newFile(const QString& content);
	void open();
	void save();
	void close();
	void refresh();
	bool canClose();

private slots:
	void connectionStateChanged() {}

private:
	void getChannel();

	RemoteConnection* mConnection;
	FTPChannel* mChannel;
	SshHost* mHost;
};

#endif // FTPFILE_H
