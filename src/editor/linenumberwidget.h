#ifndef LINENUMBERWIDGET_H
#define LINENUMBERWIDGET_H

#include <QPaintEvent>
#include <QSize>
#include <QWidget>

class CodeEditor;

class LineNumberWidget : public QWidget {
	public:
		explicit LineNumberWidget( CodeEditor *editor = nullptr );
		QSize sizeHint() const;

	protected:
		void paintEvent( QPaintEvent *event );

	private:
		CodeEditor *mEditor;

	signals:
	public slots:
};

#endif  // LINENUMBERWIDGET_H
