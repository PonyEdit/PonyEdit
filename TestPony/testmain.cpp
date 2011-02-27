#include "globals.h"
#include "itest_localfiles.h"

PonyEdit* gPonyApp;

int main(int argc, char* argv[])
{
	QCoreApplication::setOrganizationName("Pentalon");
	QCoreApplication::setApplicationName("PonyEdit Test Suite");

	gPonyApp = new PonyEdit(argc, argv);
	int result = QTest::qExec(ITestLocalFiles().self(), argc, argv);
	delete gPonyApp;

	return result;
}
