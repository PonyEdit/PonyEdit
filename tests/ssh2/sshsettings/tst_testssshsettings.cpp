#include <QByteArray>
#include <QMap>
#include <QString>
#include <QtTest>

#include "ssh2/sshsettings.h"

typedef QMap< QString, QMap< QString, QString > > configMap;

class TestsSshSettings : public QObject {
	Q_OBJECT

	public:
		TestsSshSettings();

	private Q_SLOTS:
		void initTestCase();
		void cleanupTestCase();

		void testParse_data();
		void testParse();

		void testHostname_data();
		void testHostname();
};

TestsSshSettings::TestsSshSettings() {}

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
	expectedConfig["*"]["user"] = "pento";

	QTest::newRow( "simple" )
	        << QString( "User pento" )
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig["ponyedit.com"]["hostname"] = "ponyedit.com";

	QTest::newRow( "single host" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com" )
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig["ponyedit.com"]["hostname"] = "ponyedit.com";
	expectedConfig["ponyedit.org"]["hostname"] = "ponyedit.org";

	QTest::newRow( "multi host" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com\n\n"
	                    "Host ponyedit.org\n"
	                    "    Hostname ponyedit.org" )
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig["ponyedit.com"]["hostname"] = "ponyedit.com";
	expectedConfig["ponyedit.com"]["user"] = "pento";
	expectedConfig["ponyedit.org"]["hostname"] = "ponyedit.org";
	expectedConfig["ponyedit.org"]["identitiesonly"] = "yes";

	QTest::newRow( "multi host, multi key" )
	        << QString( "Host ponyedit.com\n"
	                    "    Hostname ponyedit.com\n"
	                    "    User pento\n\n"
	                    "Host ponyedit.org\n"
	                    "    Hostname ponyedit.org\n"
	                    "    IdentitiesOnly yes" )
	        << expectedConfig;

	expectedConfig.clear();
	expectedConfig["ponyedit.com"]["hostname"] = "ponyedit.com";
	expectedConfig["ponyedit.com"]["user"] = "pento";
	expectedConfig["ponyedit.org"]["hostname"] = "ponyedit.org";
	expectedConfig["ponyedit.org"]["identitiesonly"] = "yes";
	expectedConfig["*"]["identityfile"] = "~/.ssh/id_rsa";

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

QTEST_APPLESS_MAIN( TestsSshSettings )

#include "tst_testssshsettings.moc"
