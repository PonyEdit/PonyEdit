#ifndef SSHFILE_H
#define SSHFILE_H

#include "file.h"
#include "sshremotecontroller.h"

class SshFile : public File
{
public:
	SshFile(SshRemoteController* controller, int bufferId, const QByteArray& data);

	void changeDocument(int position, int removeChars, const QByteArray& insert);

private:
	SshRemoteController* mController;
	int mBufferId;
};

#endif // SSHFILE_H
