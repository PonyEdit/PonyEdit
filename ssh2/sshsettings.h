#ifndef SSHSETTINGS_H
#define SSHSETTINGS_H

#include <QMap>
#include <QString>
#include <QStringList>

#include "sshsession.h"

class SshSettings {
	public:
		static void init();
		static SshSession::AuthMethods authMethods( QString hostname );

	private:
		static QMap< QString, QMap< QString, QString > > sConfig;
};

#endif // SSHSETTINGS_H
