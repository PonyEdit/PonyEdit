#include "file.h"

File::File(const QByteArray &data)
{
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
