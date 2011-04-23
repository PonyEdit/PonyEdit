#ifndef FTPCHANNEL_H
#define FTPCHANNEL_H

#include "remotechannel.h"
#include "rawsshconnection.h"

class FtpFile;
class FTPChannel : public RemoteChannel
{
public:
	FTPChannel(RemoteConnection* connection);

	bool hasPendingRequestsFor(FtpFile* file);

protected:
	virtual void threadConnect();
	virtual void threadSendMessages(QList<RemoteRequest*>& messages);
};

#endif // FTPCHANNEL_H
