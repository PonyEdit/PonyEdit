#include <QByteArray>
#include <QMap>
#include <QString>
#include <QtTest>

#include <QsLog.h>

#include "ssh2/sshsession.h"
#include "ssh2/sshsettings.h"

Q_DECLARE_METATYPE( SshSession::AuthMethods );

class TestsSshSettings : public QObject {
	Q_OBJECT

	public:
		TestsSshSettings();

	private Q_SLOTS:
		void initTestCase();
		void cleanupTestCase();

		void testParse_data();
		void testParse();

		void testAuthMethods_data();
		void testAuthMethods();

		void testHostname_data();
		void testHostname();

		void testUser_data();
		void testUser();

		void testIdentitiesOnly_data();
		void testIdentitiesOnly();

		void testIdentityFile_data();
		void testIdentityFile();

		void testServerAliveInternal_data();
		void testServerAliveInternal();
};

TestsSshSettings::TestsSshSettings() {
	// Ensure logging lines are run
	QsLogging::Logger &logger = QsLogging::Logger::instance();
	logger.setLoggingLevel( QsLogging::TraceLevel );
}

void TestsSshSettings::initTestCase() {}

void TestsSshSettings::cleanupTestCase() {}

void TestsSshSettings::testParse_data() {
	QTest::addColumn< QString >( "inputFile" );
	QTest::addColumn< configMap >( "expectedConfig" );

	configMap expectedConfig;
	expectedConfig.clear();

	QTest::newRow( "empty" )
	        << QString()
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig[ "*" ][ "user" ] = "pento";

	QTest::newRow( "simple" )
	        << QString( "User pento" )
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig[ "ponyedit.com" ][ "hostname" ] = "ponyedit.com";

	QTest::newRow( "single host" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com" )
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig[ "ponyedit.com" ][ "hostname" ] = "ponyedit.com";

	QTest::newRow( "single host, empty missing" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com\n"
	                    "    User" )
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig[ "ponyedit.com" ][ "hostname" ] = "ponyedit.com";
	expectedConfig[ "ponyedit.org" ][ "hostname" ] = "ponyedit.org";

	QTest::newRow( "multi host" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com\n\n"
	                    "Host ponyedit.org\n"
	                    "    Hostname ponyedit.org" )
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig[ "ponyedit.com" ][ "hostname" ]       = "ponyedit.com";
	expectedConfig[ "ponyedit.com" ][ "user" ]           = "pento";
	expectedConfig[ "ponyedit.org" ][ "hostname" ]       = "ponyedit.org";
	expectedConfig[ "ponyedit.org" ][ "identitiesonly" ] = "yes";

	QTest::newRow( "multi host, multi key" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com\n"
	                    "    User pento\n\n"
	                    "Host ponyedit.org\n"
	                    "    Hostname ponyedit.org\n"
	                    "    IdentitiesOnly yes" )
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig[ "ponyedit.com" ][ "hostname" ]       = "ponyedit.com";
	expectedConfig[ "ponyedit.com" ][ "user" ]           = "pento";
	expectedConfig[ "ponyedit.org" ][ "hostname" ]       = "ponyedit.org";
	expectedConfig[ "ponyedit.org" ][ "identitiesonly" ] = "yes";
	expectedConfig[ "*" ][ "identityfile" ]              = "~/.ssh/id_rsa";

	QTest::newRow( "multi host, multi key, top level" )
	        << QString( "IdentityFile ~/.ssh/id_rsa\n\n"
	                    "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com\n"
	                    "    User pento\n\n"
	                    "Host ponyedit.org\n"
	                    "    Hostname ponyedit.org\n"
	                    "    IdentitiesOnly yes" )
	        << expectedConfig;
}

void TestsSshSettings::testParse() {
	QFETCH( QString, inputFile );
	QFETCH( configMap, expectedConfig );

	SshSettings settings;
	settings.parse( inputFile );

	QCOMPARE( settings.getConfig(), expectedConfig );
}

void TestsSshSettings::testAuthMethods_data() {
	QTest::addColumn< QString >( "inputFile" );
	QTest::addColumn< QByteArray >( "hostname" );
	QTest::addColumn< SshSession::AuthMethods >( "expectedAuthMethods" );

	SshSession::AuthMethods expectedAuthMethods;

	expectedAuthMethods = SshSession::AuthMethod::AuthPassword | SshSession::AuthMethod::AuthKeyboardInteractive |
	                      SshSession::AuthMethod::AuthPublicKey;

	QTest::newRow( "empty" )
	        << QString()
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;

	QTest::newRow( "simple" )
	        << QString( "User pento" )
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;

	QTest::newRow( "single host, no auth" )
	        << QString( "Host ponyedit.com\n"
	                    "    User pento" )
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;

	expectedAuthMethods = SshSession::AuthMethod::AuthKeyboardInteractive | SshSession::AuthMethod::AuthPublicKey;

	QTest::newRow( "single host, disallow password" )
	        << QString( "Host ponyedit.com\n"
	                    "    PasswordAuthentication no" )
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;

	expectedAuthMethods = SshSession::AuthMethod::AuthPassword | SshSession::AuthMethod::AuthPublicKey;

	QTest::newRow( "single host, disallow keyboard" )
	        << QString( "Host ponyedit.com\n"
	                    "    KbdInteractiveAuthentication no" )
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;

	expectedAuthMethods = SshSession::AuthMethod::AuthPassword | SshSession::AuthMethod::AuthKeyboardInteractive;

	QTest::newRow( "single host, disallow public key" )
	        << QString( "Host ponyedit.com\n"
	                    "    PubkeyAuthentication no" )
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;

	expectedAuthMethods = SshSession::AuthMethod::AuthPassword;

	QTest::newRow( "single host, disallow multiple" )
	        << QString( "Host ponyedit.com\n"
	                    "    PubkeyAuthentication no\n"
	                    "    KbdInteractiveAuthentication no" )
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;

	expectedAuthMethods = SshSession::AuthMethod::AuthKeyboardInteractive | SshSession::AuthMethod::AuthPublicKey;

	QTest::newRow( "multi host, disallow password" )
	        << QString( "Host ponyedit.com\n"
	                    "    PasswordAuthentication no\n\n"
	                    "Host ponyedit.org\n"
	                    "    KbdInteractiveAuthentication no" )
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;

	expectedAuthMethods = SshSession::AuthMethod::AuthPublicKey;

	QTest::newRow( "wildcard, disallow password and keyboard" )
	        << QString( "Host ponyedit.com\n"
	                    "    PasswordAuthentication no\n\n"
	                    "Host ponyedit.*\n"
	                    "    KbdInteractiveAuthentication no" )
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;

	expectedAuthMethods = SshSession::AuthMethod::AuthPublicKey;

	QTest::newRow( "top level, disallow password and keyboard" )
	        << QString( "PasswordAuthentication no\n"
	                    "KbdInteractiveAuthentication no" )
	        << QByteArray( "ponyedit.com" )
	        << expectedAuthMethods;
}

void TestsSshSettings::testAuthMethods() {
	QFETCH( QString, inputFile );
	QFETCH( QByteArray, hostname );
	QFETCH( SshSession::AuthMethods, expectedAuthMethods );

	SshSettings settings;
	settings.parse( inputFile );

	QCOMPARE( settings.authMethods( hostname ), expectedAuthMethods );
}

void TestsSshSettings::testHostname_data() {
	QTest::addColumn< QString >( "inputFile" );
	QTest::addColumn< QByteArray >( "hostname" );
	QTest::addColumn< QByteArray >( "expectedHostname" );

	QTest::newRow( "empty" )
	        << QString()
	        << QByteArray( "ponyedit.com" )
	        << QByteArray( "ponyedit.com" );

	QTest::newRow( "single host, no hostname" )
	        << QString( "Host ponyedit.com\n"
	                    "    User pento" )
	        << QByteArray( "ponyedit.com" )
	        << QByteArray( "ponyedit.com" );

	QTest::newRow( "single host, same hostname" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com" )
	        << QByteArray( "ponyedit.com" )
	        << QByteArray( "ponyedit.com" );

	QTest::newRow( "single host, different hostname" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.org" )
	        << QByteArray( "ponyedit.com" )
	        << QByteArray( "ponyedit.org" );

	QTest::newRow( "different host, different hostname" )
	        << QString( "Host ponyedit.org\n"
	                    "    Hostname ponyedit.org" )
	        << QByteArray( "ponyedit.com" )
	        << QByteArray( "ponyedit.com" );

	QTest::newRow( "wildcard host, same hostname" )
	        << QString( "Host ponyedit.*\n"
	                    "    Hostname ponyedit.com" )
	        << QByteArray( "ponyedit.com" )
	        << QByteArray( "ponyedit.com" );

	QTest::newRow( "wildcard host, different hostname" )
	        << QString( "Host ponyedit.*\n"
	                    "    Hostname ponyedit.org" )
	        << QByteArray( "ponyedit.com" )
	        << QByteArray( "ponyedit.org" );

	QTest::newRow( "top level hostname, different hostname" )
	        << QString( "Hostname ponyedit.org" )
	        << QByteArray( "ponyedit.com" )
	        << QByteArray( "ponyedit.org" );
}

void TestsSshSettings::testHostname() {
	QFETCH( QString, inputFile );
	QFETCH( QByteArray, hostname );
	QFETCH( QByteArray, expectedHostname );

	SshSettings settings;
	settings.parse( inputFile );

	QCOMPARE( settings.hostname( hostname ), expectedHostname );
}

void TestsSshSettings::testUser_data() {
	QTest::addColumn< QString >( "inputFile" );
	QTest::addColumn< QByteArray >( "user" );
	QTest::addColumn< QByteArray >( "expectedUser" );

	QTest::newRow( "empty" )
	        << QString()
	        << QByteArray( "pento" )
	        << QByteArray( "pento" );


	QTest::newRow( "single host, no user" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com" )
	        << QByteArray( "pento" )
	        << QByteArray( "pento" );

	QTest::newRow( "single host, same user" )
	        << QString( "Host ponyedit.com\n"
	                    "    User pento" )
	        << QByteArray( "pento" )
	        << QByteArray( "pento" );

	QTest::newRow( "single host, different user" )
	        << QString( "Host ponyedit.com\n"
	                    "    User thingalon" )
	        << QByteArray( "pento" )
	        << QByteArray( "thingalon" );

	QTest::newRow( "different host, different user" )
	        << QString( "Host ponyedit.org\n"
	                    "    User thingalon" )
	        << QByteArray( "pento" )
	        << QByteArray( "pento" );

	QTest::newRow( "wildcard host, same user" )
	        << QString( "Host ponyedit.*\n"
	                    "    User pento" )
	        << QByteArray( "pento" )
	        << QByteArray( "pento" );

	QTest::newRow( "wildcard host, different user" )
	        << QString( "Host ponyedit.*\n"
	                    "    User thingalon" )
	        << QByteArray( "pento" )
	        << QByteArray( "thingalon" );

	QTest::newRow( "top level hostname, different user" )
	        << QString( "User thingalon" )
	        << QByteArray( "pento" )
	        << QByteArray( "thingalon" );
}

void TestsSshSettings::testUser() {
	QFETCH( QString, inputFile );
	QFETCH( QByteArray, user );
	QFETCH( QByteArray, expectedUser );

	SshSettings settings;
	settings.parse( inputFile );

	QCOMPARE( settings.user( QByteArray( "ponyedit.com" ), user ), expectedUser );
}

void TestsSshSettings::testIdentitiesOnly_data() {
	QTest::addColumn< QString >( "inputFile" );
	QTest::addColumn< bool >( "expectedIdentitiesSettings" );

	QTest::newRow( "empty" )
	        << QString()
	        << false;

	QTest::newRow( "single host, no setting" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com" )
	        << false;

	QTest::newRow( "single host, setting off" )
	        << QString( "Host ponyedit.com\n"
	                    "    IdentitiesOnly no" )
	        << false;

	QTest::newRow( "single host, setting on" )
	        << QString( "Host ponyedit.com\n"
	                    "    IdentitiesOnly yes" )
	        << true;

	QTest::newRow( "single host, setting gibberish" )
	        << QString( "Host ponyedit.com\n"
	                    "    IdentitiesOnly WAT" )
	        << false;

	QTest::newRow( "different host, setting on" )
	        << QString( "Host ponyedit.org\n"
	                    "    IdentitiesOnly yes" )
	        << false;

	QTest::newRow( "wildcard host, setting off" )
	        << QString( "Host ponyedit.*\n"
	                    "    IdentitiesOnly no" )
	        << false;

	QTest::newRow( "wildcard host, setting on" )
	        << QString( "Host ponyedit.*\n"
	                    "    IdentitiesOnly yes" )
	        << true;

	QTest::newRow( "top level hostname, setting on" )
	        << QString( "IdentitiesOnly yes" )
	        << true;
}

void TestsSshSettings::testIdentitiesOnly() {
	QFETCH( QString, inputFile );
	QFETCH( bool, expectedIdentitiesSettings );

	SshSettings settings;
	settings.parse( inputFile );

	QCOMPARE( settings.identitiesOnly( QByteArray( "ponyedit.com" ) ), expectedIdentitiesSettings );
}

void TestsSshSettings::testIdentityFile_data() {
	QTest::addColumn< QString >( "inputFile" );
	QTest::addColumn< QByteArray >( "identityFile" );
	QTest::addColumn< QByteArray >( "expectedIdentityFile" );

	QTest::newRow( "empty" )
	        << QString()
	        << QByteArray( "~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_rsa" );


	QTest::newRow( "single host, no IdentityFile" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com" )
	        << QByteArray( "~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_rsa" );

	QTest::newRow( "single host, same IdentityFile" )
	        << QString( "Host ponyedit.com\n"
	                    "    IdentityFile ~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_rsa" );

	QTest::newRow( "single host, different IdentityFile" )
	        << QString( "Host ponyedit.com\n"
	                    "    IdentityFile ~/.ssh/id_md5" )
	        << QByteArray( "~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_md5" );

	QTest::newRow( "different host, different IdentityFile" )
	        << QString( "Host ponyedit.org\n"
	                    "    IdentityFile ~/.ssh/id_md5" )
	        << QByteArray( "~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_rsa" );

	QTest::newRow( "wildcard host, same IdentityFile" )
	        << QString( "Host ponyedit.*\n"
	                    "    IdentityFile ~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_rsa" );

	QTest::newRow( "wildcard host, different IdentityFile" )
	        << QString( "Host ponyedit.*\n"
	                    "    IdentityFile ~/.ssh/id_md5" )
	        << QByteArray( "~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_md5" );

	QTest::newRow( "top level hostname, different IdentityFile" )
	        << QString( "IdentityFile ~/.ssh/id_md5" )
	        << QByteArray( "~/.ssh/id_rsa" )
	        << QByteArray( "~/.ssh/id_md5" );
}

void TestsSshSettings::testIdentityFile() {
	QFETCH( QString, inputFile );
	QFETCH( QByteArray, identityFile );
	QFETCH( QByteArray, expectedIdentityFile );

	SshSettings settings;
	settings.parse( inputFile );

	QCOMPARE( settings.identityFile( QByteArray( "ponyedit.com" ), identityFile ), expectedIdentityFile );
}

void TestsSshSettings::testServerAliveInternal_data() {
	QTest::addColumn< QString >( "inputFile" );
	QTest::addColumn< int >( "expectedServerAliveInternal" );

	QTest::newRow( "empty" )
	        << QString()
	        << 0;

	QTest::newRow( "single host, no setting" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com" )
	        << 0;

	QTest::newRow( "single host, setting off" )
	        << QString( "Host ponyedit.com\n"
	                    "    ServerAliveInternal 0" )
	        << 0;

	QTest::newRow( "single host, setting on" )
	        << QString( "Host ponyedit.com\n"
	                    "    ServerAliveInternal 42" )
	        << 42;

	QTest::newRow( "single host, setting gibberish" )
	        << QString( "Host ponyedit.com\n"
	                    "    ServerAliveInternal WAT" )
	        << 0;

	QTest::newRow( "different host, setting on" )
	        << QString( "Host ponyedit.org\n"
	                    "    ServerAliveInternal 42" )
	        << 0;

	QTest::newRow( "wildcard host, setting off" )
	        << QString( "Host ponyedit.*\n"
	                    "    ServerAliveInternal 0" )
	        << 0;

	QTest::newRow( "wildcard host, setting on" )
	        << QString( "Host ponyedit.*\n"
	                    "    ServerAliveInternal 42" )
	        << 42;

	QTest::newRow( "top level hostname, setting on" )
	        << QString( "ServerAliveInternal 42" )
	        << 42;
}

void TestsSshSettings::testServerAliveInternal() {
	QFETCH( QString, inputFile );
	QFETCH( int, expectedServerAliveInternal );

	SshSettings settings;
	settings.parse( inputFile );

	QCOMPARE( settings.serverAliveInterval( QByteArray( "ponyedit.com" ) ), expectedServerAliveInternal );
}

QTEST_APPLESS_MAIN( TestsSshSettings )

#include "tst_testssshsettings.moc"
