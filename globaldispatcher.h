#ifndef GLOBALDISPATCHER_H
#define GLOBALDISPATCHER_H

#include <QObject>
#include <QString>

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
	void emitActiveFilesUpdated() { emit activeFilesUpdated(); }
	void emitGeneralErrorMessage(QString error) { emit generalErrorMessage(error); }
	void emitGeneralStatusMessage(QString message) { emit generalStatusMessage(message); }

signals:
	void sshServersUpdated();
	void activeFilesUpdated();
	void generalErrorMessage(QString error);
	void generalStatusMessage(QString message);
};

extern GlobalDispatcher* gDispatcher;

#endif // GLOBALDISPATCHER_H
