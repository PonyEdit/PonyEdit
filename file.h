#ifndef FILE_H
#define FILE_H

#include <QByteArray>

class File
{
public:
	File(const QByteArray& data);

	inline const QByteArray& getData() const { return mData; }

	virtual void changeDocument(int position, int removeChars, const QByteArray& insert);
	virtual void save() = 0;

protected:
	QByteArray mData;
	bool mChanged;
	int mRevision;
	int mLastSavedRevision;
};

#endif // FILE_H
