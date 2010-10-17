#ifndef QSSHFILEENGINE_H
#define QSSHFILEENGINE_H

#include <QAbstractFileEngine>

class SshFileEngine : public QAbstractFileEngine
{
public:
	SshFileEngine();
};


//
//	Handler class; used to determine when to show this file handler
//

class SshFileEngineHandler : public QAbstractFileEngineHandler
{
public:
	QAbstractFileEngine* create(const QString& filename) const;
};


#endif // QSSHFILEENGINE_H
