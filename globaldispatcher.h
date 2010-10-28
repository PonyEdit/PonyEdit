#ifndef GLOBALDISPATCHER_H
#define GLOBALDISPATCHER_H

#include <QObject>

//
//	The global dispatch object (gDispatcher) is used to dispatch globally interesting signals
//	eg; "SSH hosts list has been updated"
//

class GlobalDispatcher : public QObject
{
	Q_OBJECT

public:
	GlobalDispatcher() {}	//	Do not call outside of main(); just use gDispatcher instead.
	void emitSshServersUpdated() { emit sshServersUpdated(); }

signals:
	void sshServersUpdated();
};

extern GlobalDispatcher* gDispatcher;

#endif // GLOBALDISPATCHER_H
