#include "file.h"
#include <QDebug.h>

File::File(const Location& location, const QByteArray &data)
{
	//	Detect line ending mode, then convert to unix-style. Use unix-style line endings everywhere,
	//	only convert to DOS on saving if that's what we're coming from.
	mDosLineEndings = mData.contains("\r\n");
	if (mDosLineEndings)
		mData.replace("\r\n", "\n");

	mLocation = location;
	mData = data;
	mChanged = false;
	mRevision = 0;
	mLastSavedRevision = 0;
}

void File::changeDocument(int position, int removeChars, const QByteArray& insert)
{
	mRevision++;
	mData.replace(position, removeChars, insert);
	mChanged = true;
}
