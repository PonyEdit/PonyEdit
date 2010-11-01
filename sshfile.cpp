#include "sshfile.h"
#include "sshrequest.h"

SshFile::SshFile(SshRemoteController* controller, int bufferId, const QByteArray& data) : File(data)
{
	mBufferId = bufferId;
	mController = controller;
}

void SshFile::changeDocument(int position, int removeChars, const QByteArray& insert)
{
	File::changeDocument(position, removeChars, insert);


}
