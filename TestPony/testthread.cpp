#include "testthread.h"
#include "itestsshfiles.h"
#include <QTest>

TestThread::TestThread(int argc, char* argv[]) :
	QThread(NULL)
{
	mArgc = argc;
	mArgv = argv;
}


void TestThread::run()
{
	mResult = QTest::qExec(ITestSshFiles().self(), mArgc, mArgv);
}
