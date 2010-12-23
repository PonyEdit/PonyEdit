#include "unsavedfile.h"

UnsavedFile::UnsavedFile(const Location& location) :
	BaseFile(location)
{
}

void UnsavedFile::newFile()
{
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
}
