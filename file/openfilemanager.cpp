#include <QDebug>

#include "openfilemanager.h"
#include "unsavedchangesdialog.h"
#include "basefile.h"

OpenFileManager gOpenFileManager;

OpenFileManager::OpenFileManager() : QObject(0)
{
		mNewFiles = 1;
}

BaseFile* OpenFileManager::getFile(const Location& location) const
{
	foreach (BaseFile* file, mOpenFiles)
		if (location.getProtocol() != Location::Unsaved && file->getLocation() == location)
			return file;

	return NULL;
}

void OpenFileManager::registerFile(BaseFile* file)
{
	mOpenFiles.append(file);
	emit fileOpened(file);
}

void OpenFileManager::deregisterFile(BaseFile* file)
{
	if (mOpenFiles.removeAll(file))
		emit fileClosed(file);
}

void OpenFileManager::reregisterFile(BaseFile* file)
{
	deregisterFile(file);
	reregisterFile(file);
}

const QList<BaseFile*> OpenFileManager::getOpenFiles() const
{
	return mOpenFiles;
}

bool OpenFileManager::closeFiles(const QList<BaseFile*>& files, bool /* force */)
{
	QList<BaseFile*> unsavedFiles = getUnsavedFiles(files);
	if (unsavedFiles.length() > 0)
	{
		UnsavedChangesDialog dialog(unsavedFiles);
		if (dialog.exec() != QDialog::Accepted)
			return false;
	}

	foreach (BaseFile* file, files)
	{
		if (mOpenFiles.contains(file))
		{
			try
			{
				file->close();
			}
			catch(QString &e)
			{
				qDebug() << e;
			}

			deregisterFile(file);
		}
	}

	return true;
}

bool OpenFileManager::unsavedChanges() const
{
	foreach (BaseFile* file, mOpenFiles)
		if (file->hasUnsavedChanges())
			return true;
	return false;
}

QList<BaseFile*> OpenFileManager::getUnsavedFiles(const QList<BaseFile*>& files) const
{
	QList<BaseFile*> result;
	foreach (BaseFile* file, files)
		if (file->hasUnsavedChanges())
			result.append(file);
	return result;
}
