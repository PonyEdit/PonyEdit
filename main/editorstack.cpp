#include "editorstack.h"
#include "editor/editor.h"
#include "editorpanel.h"
#include "windowmanager.h"

EditorStack::EditorStack( EditorPanel *parent )
    : QStackedWidget( parent ) {
	mParentPanel = parent;
}

Editor *EditorStack::getCurrentEditor() const {
	return static_cast< Editor * >( currentWidget() );
}

void EditorStack::displayEditor( Editor *editor ) {
	//	Nothing to do if already displayed.
	if ( currentWidget() == editor )
		return;

	setCurrentWidget( editor );
	gWindowManager->notifyEditorChanged( mParentPanel );
}

void EditorStack::displayFile( BaseFile *file ) {
	//	First, see if there is already an editor in this stack for the file...
	foreach ( Editor *editor, mEditors ) {
		if ( file->getLocation().getProtocol() == Location::Unsaved ) {
			if ( editor->getLocation().getLabel() == file->getLocation().getLabel() ) {
				displayEditor( editor );
				return;
			}
		} else if ( editor->getLocation() == file->getLocation() ) {
			displayEditor( editor );
			return;
		}
	}

	//	If not, create a new editor for the file.
	createEditor( file );
}

void EditorStack::createEditor( BaseFile *file ) {
	Editor *newEditor = new Editor( file );
	mEditors.append( newEditor );
	addWidget( newEditor );

	displayEditor( newEditor );
}

void EditorStack::fileClosed( BaseFile *file ) {
	for ( int i = 0; i < mEditors.length(); i++ ) {
		if ( mEditors[ i ]->getFile() == file ) {
			bool current = ( getCurrentEditor() == mEditors[ i ] );
			mEditors.removeAt( i );
			if ( current )
				gWindowManager->notifyEditorChanged( mParentPanel );
			return;
		}
	}
}
