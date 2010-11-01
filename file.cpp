#include "file.h"

File::File(const QByteArray &data)
{
	mData = data;
	mChanged = false;
}

void File::changeDocument(int position, int removeChars, const QByteArray& insert)
{
	mData.replace(position, removeChars, insert);
	mChanged = true;
}
