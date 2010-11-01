#ifndef FILE_H
#define FILE_H

#include <QByteArray>

class File
{
public:
	File(const QByteArray& data);

	inline const QByteArray& getData() const { return mData; }

protected:
	QByteArray mData;
};

#endif // FILE_H
