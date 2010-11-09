#ifndef SSHFILE_H
#define SSHFILE_H

#include "basefile.h"

class SshRemoteController;
class SshFile : public BaseFile
{
public:
	void open();
	void save();
	void savedRevision(int revision);
	void fileOpened(int bufferId, const QByteArray& content);

	SshFile(const Location& location);	//	Do not call; use File::getFile instead.

protected:
	virtual ~SshFile();
	virtual void handleDocumentChange(int position, int removeChars, const QByteArray& insert);

	SshHost* mHost;
	int mBufferId;
};

#endif // SSHFILE_H
