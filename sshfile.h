#ifndef SSHFILE_H
#define SSHFILE_H

#include "file.h"

class SshFile : public File
{
public:
	SshFile(int bufferId, const QByteArray& data);

private:
	int mBufferId;
};

#endif // SSHFILE_H
