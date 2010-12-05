#ifndef OPENFILEMANAGER_H
#define OPENFILEMANAGER_H

#include <QObject>
#include <QList>

#include "location.h"

class BaseFile;

class OpenFileManager : public QObject
{
    Q_OBJECT
public:
	explicit OpenFileManager();

	BaseFile* getFile(const Location& location) const;   // Returns already-open files at specified location, NULL if not opened already
	void registerFile(BaseFile* file);                   // Register the specified file with the manager. Manager then takes ownership and manages object lifecycle
	const QList<BaseFile*> getOpenFiles() const;         // Returns a list of all the currently opened files

signals:
	void fileOpened(BaseFile* file);   // Emitted every time a file is opened
	void fileClosed(BaseFile* file);   // Emitted as each file is closed; immediately before object deletion

private:
	QList<BaseFile*> mOpenFiles;
};

extern OpenFileManager gOpenFileManager;

#endif // OPENFILEMANAGER_H
