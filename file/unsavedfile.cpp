#include "unsavedfile.h"

UnsavedFile::UnsavedFile( const Location& location ) :
	BaseFile( location )
{}

BaseFile* UnsavedFile::newFile( const QString& /* content */ ) {
	setOpenStatus( Ready );
	return this;
}

void UnsavedFile::open() {
	setOpenStatus( Ready );
}

void UnsavedFile::save() {
	setOpenStatus( Ready );
}

void UnsavedFile::close() {
	setOpenStatus( Closing );
	closeCompleted();
}

void UnsavedFile::refresh()
{}
