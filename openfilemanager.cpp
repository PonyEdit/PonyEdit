#include "openfilemanager.h"

#include "basefile.h"

OpenFileManager gOpenFileManager;

OpenFileManager::OpenFileManager() : QObject(0)
{
}

BaseFile* OpenFileManager::getFile(const Location& location) const
{
	foreach (BaseFile* file, mOpenFiles)
		if (file->getLocation() == location)
			return file;

	return NULL;
}

void OpenFileManager::registerFile(BaseFile* file)
{
	mOpenFiles.append(file);
	emit fileOpened(file);
}

const QList<BaseFile*> OpenFileManager::getOpenFiles() const
{
	return mOpenFiles;
}

