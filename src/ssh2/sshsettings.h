#ifndef SSHSETTINGS_H
#define SSHSETTINGS_H

#include <QByteArray>
#include <QMap>
#include <QString>
#include <QStringList>

#include "sshsession.h"

typedef QMap< QString, QMap< QString, QString > > configMap;

class SshSettings {
	public:
		SshSettings();
		void parse( QString config );

		SshSession::AuthMethods authMethods( QByteArray hostname );

		QByteArray hostname( QByteArray hostname ) {
			return getValue( hostname, "hostname", hostname ).toLatin1();
		}

		QByteArray user( QByteArray hostname, QByteArray user ) {
			return getValue( hostname, "user", user ).toLatin1();
		}

		bool identitiesOnly( QByteArray hostname ) {
			return ( getValue( hostname, "identitiesonly", "no" ) == "yes" );
		}

		QByteArray identityFile( QByteArray hostname, QByteArray identityFile ) {
			return getValue( hostname, "identityfile", identityFile ).toLatin1();
		}

		int serverAliveInterval( QByteArray hostname ) {
			return getValue( hostname, "serveraliveinternal", "0" ).toInt();
		}

		QMap< QString, QMap< QString, QString > > getConfig() {
			return mConfig;
		}

	protected:
		QString getValue( QByteArray hostname, QString key, QByteArray originalValue );

	private:
		/*
		 * mConfig is stored like so:
		 * {
		 *   hostname: {
		 *     setting: value
		 *     setting: value
		 *   },
		 *   hostname: {
		 *     ...
		 *   },
		 *   ...
		 *
		 * hostname can be either a static string, or a Unix wildcard matcher
		 */
		configMap mConfig;
};

#endif // SSHSETTINGS_H
