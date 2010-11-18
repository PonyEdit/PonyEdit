#ifndef GLOBALDISPATCHER_H
#define GLOBALDISPATCHER_H

#include <QObject>
#include <QString>
#include "location.h"

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

	void emitLocationListSuccessful(const QList<Location>& children, QString locationPath) { emit locationListSuccessful(children, locationPath); }
	void emitLocationListFailed(const QString& error, QString locationPath) { emit locationListFailed(error, locationPath); }

signals:
	void sshServersUpdated();
	void activeFilesUpdated();

	void generalErrorMessage(QString error);
	void generalStatusMessage(QString message);

	void locationListSuccessful(const QList<Location>& children, QString locationPath);
	void locationListFailed(const QString& error, QString locationPath);
};

extern GlobalDispatcher* gDispatcher;

#endif // GLOBALDISPATCHER_H
