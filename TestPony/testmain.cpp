#include "globals.h"
#include "testthread.h"
#include <QSettings>

PonyEdit* gPonyApp;
QString gHost;
int gPort;

int main(int argc, char* argv[])
{
	QCoreApplication::setOrganizationName("Pentalon");
	QCoreApplication::setApplicationName("PonyEdit Test Suite");

	QSettings iniSettings("testpony.ini", QSettings::IniFormat);
	gHost = iniSettings.value("host", "localhost").toString();
	gPort = iniSettings.value("port", 2222).toInt();
	iniSettings.setValue("host", "localhost");
	iniSettings.setValue("port", "2222");

	gPonyApp = new PonyEdit(argc, argv);

	TestThread* testThread = new TestThread(argc, argv);
	testThread->start();
	testThread->wait();
	int result = testThread->getResult();

	delete testThread;
	delete gPonyApp;
	return result;
}
