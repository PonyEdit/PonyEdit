#include "unsavedfile.h"

UnsavedFile::UnsavedFile(const Location& location) :
	BaseFile(location)
{
}

BaseFile* UnsavedFile::newFile(const QByteArray& /* content */)
{
	return this;
}

void UnsavedFile::open()
{
}

void UnsavedFile::save()
{
	setOpenStatus(Ready);
}

void UnsavedFile::close()
{
	setOpenStatus(Closing);
	closeCompleted();
}
