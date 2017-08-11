#ifndef GLOBALDISPATCHER_H
#define GLOBALDISPATCHER_H

#include <QObject>
#include <QString>
#include "file/location.h"

//
// The global dispatch object (gDispatcher) is used to dispatch globally interesting signals
// eg; "SSH hosts list has been updated"
//

class RemoteConnection;
class GlobalDispatcher : public QObject {
	Q_OBJECT

	public:
		GlobalDispatcher() {}   // Do not call outside of main(); just use gDispatcher instead.
		void emitSshServersUpdated() {
			emit sshServersUpdated();
		}

		void emitGeneralErrorMessage( QString error ) {
			emit generalErrorMessage( error );
		}

		void emitGeneralStatusMessage( QString message ) {
			emit generalStatusMessage( message );
		}

		void emitLocationListSuccess( const QList< Location >& children, QString locationPath ) {
			emit locationListSuccess( children, locationPath );
		}

		void emitLocationListFailure( const QString& error, QString locationPath, bool permissionError ) {
			emit locationListFailure( error, locationPath, permissionError );
		}

		void emitSelectFile( BaseFile* file ) {
			emit selectFile( file );
		}

		void emitSyntaxChanged( BaseFile* file ) {
			emit syntaxChanged( file );
		}

		void emitConnectionDropped( RemoteConnection* connection ) {
			emit connectionDropped( connection );
		}

		void emitOptionsChanged() {
			emit optionsChanged();
		}

	signals:
		void sshServersUpdated();

		void generalErrorMessage( QString error );
		void generalStatusMessage( QString message );

		void locationListSuccess( const QList< Location >& children, QString locationPath );
		void locationListFailure( const QString& error, QString locationPath, bool permissionError );

		void selectFile( BaseFile* file );
		void syntaxChanged( BaseFile* file );
		void connectionDropped( RemoteConnection* connection );
		void optionsChanged();
};

extern GlobalDispatcher* gDispatcher;

#endif  // GLOBALDISPATCHER_H
