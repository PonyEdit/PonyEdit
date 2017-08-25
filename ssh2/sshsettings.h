#ifndef SSHSETTINGS_H
#define SSHSETTINGS_H

#include <QMap>
#include <QString>
#include <QStringList>

#include "sshsession.h"

class SshSettings {
	public:
		SshSettings();
		SshSession::AuthMethods authMethods( QString hostname );

	private:
		QMap< QString, QMap< QString, QString > > mConfig;
};

#endif // SSHSETTINGS_H
