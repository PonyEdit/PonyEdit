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
	//	Keep the mOpenFiles list alphabetically sorted by Location.
	int scan;
	for (scan = 0; scan < mOpenFiles.length(); scan++)
		if (mOpenFiles[scan]->getLocation().getPath() > file->getLocation().getPath())
			break;
	mOpenFiles.insert(scan, file);

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
	registerFile(file);
}

const QList<BaseFile*> OpenFileManager::getOpenFiles() const
{
	return mOpenFiles;
}

bool OpenFileManager::closeFiles(const QList<BaseFile*>& files, bool force)
{
	if (!force) {
		QList<BaseFile*> unsavedFiles = getUnsavedFiles(files);
		if (unsavedFiles.length() > 0)
		{
			UnsavedChangesDialog dialog(unsavedFiles);
			if (dialog.exec() != QDialog::Accepted)
				return false;
		}
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

bool OpenFileManager::refreshFiles(const QList<BaseFile*>& files, bool force)
{
	if (!force) {
		QList<BaseFile*> unsavedFiles = getUnsavedFiles(files);
		if (unsavedFiles.length() > 0)
		{
			UnsavedChangesDialog dialog(unsavedFiles);
			if (dialog.exec() != QDialog::Accepted)
				return false;
		}

	}

	foreach (BaseFile* file, files)
	{
		if (mOpenFiles.contains(file))
		{
			try
			{
				file->refresh();
			}
			catch(QString &e)
			{
				qDebug() << e;
			}
		}
	}

	return true;
}

BaseFile* OpenFileManager::getNextFile(BaseFile* file)
{
	int index = mOpenFiles.indexOf(file);
	return (index < 0 || index == mOpenFiles.length() - 1) ? mOpenFiles.at(0) : mOpenFiles.at(index + 1);
}

BaseFile* OpenFileManager::getPreviousFile(BaseFile* file)
{
	int index = mOpenFiles.indexOf(file);
	return index < 1 ? mOpenFiles.at(0) : mOpenFiles.at(index + 1);
}



