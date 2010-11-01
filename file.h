#ifndef FILE_H
#define FILE_H

#include <QByteArray>

class File
{
public:
	File(const QByteArray& data);

	inline const QByteArray& getData() const { return mData; }

	virtual void changeDocument(int position, int removeChars, const QByteArray& insert);

protected:
	QByteArray mData;
	bool mChanged;
};

#endif // FILE_H
