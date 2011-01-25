#ifndef UNSAVEDFILE_H
#define UNSAVEDFILE_H

#include "basefile.h"

class UnsavedFile : public BaseFile
{
    Q_OBJECT
public:
	explicit UnsavedFile(const Location& location);

	BaseFile* newFile(const QString& content);
	void open();
	void save();
	void close();

signals:

public slots:

};

#endif // UNSAVEDFILE_H
