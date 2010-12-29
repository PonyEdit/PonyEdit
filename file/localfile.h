#ifndef LOCALFILE_H
#define LOCALFILE_H

#include <QFile>
#include <QTextStream>

#include "basefile.h"

class LocalFile : public BaseFile
{
	Q_OBJECT

public:
	LocalFile(const Location& location);

	BaseFile* newFile(const QByteArray& content);
	void open();
	void save();
	void close();

signals:
	void localFileOpened(const QByteArray& content);
};

#endif // LOCALFILE_H
