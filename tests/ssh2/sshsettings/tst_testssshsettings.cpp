#include <QString>
#include <QtTest>

class TestsSshSettings : public QObject {
	Q_OBJECT

	public:
		TestsSshSettings();

	private Q_SLOTS:
		void initTestCase();
		void cleanupTestCase();
		void testCase1_data();
		void testCase1();
};

TestsSshSettings::TestsSshSettings()
{}

void TestsSshSettings::initTestCase()
{}

void TestsSshSettings::cleanupTestCase()
{}

void TestsSshSettings::testCase1_data() {
	QTest::addColumn< QString >( "data" );
	QTest::newRow( "0" ) << QString();
}

void TestsSshSettings::testCase1() {
	QFETCH( QString, data );
	QVERIFY2( false, "Failure" );
}

QTEST_APPLESS_MAIN( TestsSshSettings )

#include "tst_testssshsettings.moc"
