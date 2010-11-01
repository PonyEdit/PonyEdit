#include "sshfile.h"

SshFile::SshFile(int bufferId, const QByteArray& data) : File(data)
{
	mBufferId = bufferId;
}
