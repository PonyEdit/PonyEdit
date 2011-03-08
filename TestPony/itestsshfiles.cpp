#include "itestsshfiles.h"
#include "globals.h"
#include "ssh/serverconfigdlg.h"
#include <QObject>
#include <QTest>

void ITestSshFiles::initTestCase()
{
}

void ITestSshFiles::cleanupTestCase()
{
}

void ITestSshFiles::test001_reconnectAfterRoughDisconnect()
{
	QVERIFY(1);
	QVERIFY(0);

/*	Location loc("thingalon@trouble.net.au:~/atestfile.pl");
	gMainWindow->openSingleFile(&loc);

	QWidget* currentWindow = gPonyApp->focusWidget()->window();
	QVERIFY(currentWindow->inherits("ServerConfigDlg"));*/
}

