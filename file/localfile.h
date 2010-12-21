#include <QFile>
#include <QTextStream>

#include "basefile.h"

#ifndef LOCALFILE_H
#define LOCALFILE_H

class LocalFile : public BaseFile
{
public:
	LocalFile(const Location& location);

	void open();
	void save();
	void close();

};

#endif // LOCALFILE_H
