#include <QObject>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include "globals.h"

class PonyEdit;
class ITestLocalFiles : public QObject
{
	Q_OBJECT

public:
	ITestLocalFiles() {}
	QObject* self() { return this; }

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();
	void testCase1();
};



