#ifndef SSHSETTINGS_H
#define SSHSETTINGS_H

#include <QMap>
#include <QString>
#include <QStringList>

#include "sshsession.h"

class SshSettings {
	public:
		SshSettings();
		SshSession::AuthMethods authMethods( QByteArray hostname );
		QByteArray hostname( QByteArray hostname );

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
		QMap< QString, QMap< QString, QString > > mConfig;
};

#endif // SSHSETTINGS_H
