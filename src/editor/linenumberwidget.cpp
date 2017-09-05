#include "codeeditor.h"
#include "linenumberwidget.h"

LineNumberWidget::LineNumberWidget( CodeEditor *editor ) :
	QWidget( editor ) {
	mEditor = editor;
}

QSize LineNumberWidget::sizeHint() const {
	return QSize( mEditor->lineNumberAreaWidth(), 0 );
}

void LineNumberWidget::paintEvent( QPaintEvent *event ) {
	mEditor->lineNumberAreaPaintEvent( event );
}
