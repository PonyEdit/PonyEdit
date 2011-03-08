#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <QThread>

class TestThread : public QThread
{
    Q_OBJECT
public:
	explicit TestThread(int argc, char* argv[]);
	void run();

	inline int getResult() { return mResult; }

private:
	int mResult;
	int mArgc;
	char** mArgv;
};

#endif // TESTTHREAD_H
