#ifndef ITESTSSHFILES_H
#define ITESTSSHFILES_H

#include <QObject>

class ITestSshFiles : public QObject
{
    Q_OBJECT
public:
	explicit ITestSshFiles() {}
	QObject* self() { return this; }

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void test001_reconnectAfterRoughDisconnect();
};

#endif // ITESTSSHFILES_H
