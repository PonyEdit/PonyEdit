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

	QString fileContent = file.readAll();

	file.close();

	parse( fileContent );
}

void SshSettings::parse( QString config ) {
	QTextStream in( &config );
	QString line,
	        key,
	        value,
	        currentHost = "*";

	mConfig.clear();

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
			if ( ! iterator.value()["passwordauthentication"].isEmpty() &&
			     iterator.value()["passwordauthentication"] != "yes" ) {
				methods &= ~SshSession::AuthMethod::AuthPassword;
			}

			if ( ! iterator.value()["kbdinteractiveauthentication"].isEmpty() &&
			     iterator.value()["kbdinteractiveauthentication"] != "yes" ) {
				methods &= ~SshSession::AuthMethod::AuthKeyboardInteractive;
			}

			if ( ! iterator.value()["pubkeyauthentication"].isEmpty() &&
			     iterator.value()["pubkeyauthentication"] != "yes" ) {
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
