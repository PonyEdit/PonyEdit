#include <QFile>
#include <QRegExp>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>

#include "QsLog.h"
#include "sshsettings.h"

SshSettings::SshSettings() :
	mConfig() {
	QFile file( QStandardPaths::writableLocation( QStandardPaths::HomeLocation ) + "/.ssh/config" );

	if ( ! file.open( QFile::ReadOnly | QFile::Text ) ) {
		QLOG_INFO() << "Unable to open SSH config file";
		return;
	}

	QTextStream in( &file );
	QString line, key, value, currentHost;
	while ( in.readLineInto( &line ) ) {
		line = line.trimmed();
		line.remove( QRegExp( "#.*" ) );
		if ( line.isEmpty() ) {
			continue;
		}

		key = line.section( ' ', 0, 0 ).trimmed().toLower();
		value = line.section( ' ', 1 ).trimmed().toLower();

		if ( key.isEmpty() || value.isEmpty() ) {
			continue;
		}

		if ( key == "host" ) {
			currentHost = value;
			continue;
		}

		mConfig[currentHost][key] = value;
	}
}

SshSession::AuthMethods SshSettings::authMethods( QByteArray hostname ) {
	SshSession::AuthMethods methods = SshSession::AuthMethod::AuthPassword | SshSession::AuthMethod::AuthKeyboardInteractive |
	                                  SshSession::AuthMethod::AuthPublicKey;

	hostname = hostname.toLower();

	QMap< QString, QMap< QString, QString > >::const_iterator iterator = mConfig.constBegin();

	QRegExp hostMatch;
	hostMatch.setPatternSyntax( ( QRegExp::Wildcard ) );
	while ( iterator != mConfig.constEnd() ) {
		hostMatch.setPattern( iterator.key() );
		if ( hostMatch.exactMatch( hostname ) ) {
			if ( iterator.value()["passwordauthentication"] != "yes" ) {
				methods &= ~SshSession::AuthMethod::AuthPassword;
			} else if ( iterator.value()["kbdinteractiveauthentication"] != "yes" ) {
				methods &= ~SshSession::AuthMethod::AuthKeyboardInteractive;
			} else if ( iterator.value()["pubkeyauthentication"] != "yes" ) {
				methods &= ~SshSession::AuthMethod::AuthPublicKey;
			}
		}

		++iterator;
	}

	return methods;
}

QByteArray SshSettings::hostname( QByteArray hostname ) {
	QByteArray returnHostname = hostname = hostname.toLower();

	QMap< QString, QMap< QString, QString > >::const_iterator iterator = mConfig.constBegin();

	QRegExp hostMatch;
	hostMatch.setPatternSyntax( ( QRegExp::Wildcard ) );
	while ( iterator != mConfig.constEnd() ) {
		hostMatch.setPattern( iterator.key() );
		if ( hostMatch.exactMatch( hostname ) ) {
			if ( ! iterator.value()["hostname"].isEmpty() ) {
				returnHostname = iterator.value()["hostname"].toLatin1();
			}
		}

		++iterator;
	}

	return returnHostname;
}
