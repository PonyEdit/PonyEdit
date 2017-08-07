#ifndef OPENFILEMANAGER_H
#define OPENFILEMANAGER_H

#include <QList>
#include <QObject>

#include "location.h"

class BaseFile;

class OpenFileManager : public QObject
{
Q_OBJECT
public:
explicit OpenFileManager();

BaseFile* getFile( const Location& location ) const;		// Returns already-open files at specified location,
								// NULL if not opened already
void registerFile( BaseFile* file );				// Register the specified file with the manager. Manager
								// then takes ownership and manages object lifecycle
void deregisterFile( BaseFile* file );				// Remove the specified file from the manager. Used
								// after files are successfully closed
void reregisterFile( BaseFile* file );				// If a file changes location or significantly changes
								// property, this will deregister and reregister it.
const QList< BaseFile* > getOpenFiles() const;			// Returns a list of all the currently opened files

bool unsavedChanges() const;					// Returns true if any opened file has unsaved changes
QList< BaseFile* > getUnsavedFiles( const QList< BaseFile* >& files ) const;			// Returns a list of
												// files with unsaved
												// changes

bool closeFiles( const QList< BaseFile* >& files, bool force = false );		// Closes the list of files. If force
										// unspecified or false, it confirms
										// closure of unsaved files.
bool closeAllFiles() {
	return closeFiles( mOpenFiles );
}

bool refreshFiles( const QList< BaseFile* >& files, bool force = false );

int newFileNumber() {
	return mNewFiles++;
}

inline int getFileCount() const {
	return mOpenFiles.count();
}

BaseFile* getNextFile( BaseFile* file );
BaseFile* getPreviousFile( BaseFile* file );

signals:
void fileOpened( BaseFile* file );		// Emitted every time a file is opened
void fileClosed( BaseFile* file );		// Emitted as each file is closed; immediately before object deletion

private:
QList< BaseFile* > mOpenFiles;
int mNewFiles;
};

extern OpenFileManager gOpenFileManager;

#endif	// OPENFILEMANAGER_H
