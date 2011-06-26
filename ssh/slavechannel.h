#ifndef SLAVECHANNEL_H
#define SLAVECHANNEL_H

#include "remotechannel.h"
#include "remoteconnection.h"

class SlaveChannel : public RemoteChannel
{
public:
	SlaveChannel(RemoteConnection* connection, bool sudo);

protected:
	virtual void threadConnect();
	virtual void threadSendMessages(QList<RemoteRequest*>& messages);
	virtual void threadCheckForNotifications();
	void handleNotification(const QByteArray& notification);

	bool mSudo;
};

#endif // SLAVECHANNEL_H
