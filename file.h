#ifndef FILE_H
#define FILE_H

#include <QByteArray>
#include "location.h"

class File
{
public:
	File(const Location& location, const QByteArray& data);

	inline const QByteArray& getData() const { return mData; }
	inline const Location& getLocation() const { return mLocation; }

	virtual void changeDocument(int position, int removeChars, const QByteArray& insert);
	virtual void save() = 0;

protected:
	Location mLocation;
	QByteArray mData;
	bool mChanged;
	bool mDosLineEndings;
	int mRevision;
	int mLastSavedRevision;
};

#endif // FILE_H
