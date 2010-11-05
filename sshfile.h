#ifndef SSHFILE_H
#define SSHFILE_H

#include "basefile.h"

class SshFile : public BaseFile
{
public:
	SshFile(SshRemoteController* controller, const Location& location);

	void save();
	void savedRevision(int revision);
	void fileOpened(int bufferId, const QByteArray& content);

protected:
	virtual void handleDocumentChange(int position, int removeChars, const QByteArray& insert);
	void loadContent();

	SshRemoteController* mController;
	int mBufferId;
};

#endif // SSHFILE_H
