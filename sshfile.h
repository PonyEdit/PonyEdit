#ifndef SSHFILE_H
#define SSHFILE_H

#include "basefile.h"

class SshRemoteController;
class SshFile : public BaseFile
{
	Q_OBJECT

public:
	void open();
	void save();
	void savedRevision(int revision);
	void fileOpened(int bufferId, const QByteArray& content, const QString& checksum);

	SshFile(const Location& location);	//	Do not call; use File::getFile instead.
	void reconnect();

	void contentPushError(const QString& error);
	void contentPushSuccess();

public slots:
	void connectionStateChanged();

protected:
	virtual ~SshFile();
	virtual void handleDocumentChange(int position, int removeChars, const QByteArray& insert);
	void pushContentToSlave();

	SshHost* mHost;
	int mBufferId;
};

#endif // SSHFILE_H
