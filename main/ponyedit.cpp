#include <QEvent>
#include <QFileOpenEvent>

#include "ponyedit.h"
#include "file/location.h"
#include "mainwindow.h"

PonyEdit::PonyEdit(int argc, char** argv) : QApplication(argc, argv)
{
}

bool PonyEdit::event(QEvent *e)
{
	if(e->type() == QEvent::FileOpen)
	{
		if(!gMainWindow)
			return false;

		QFileOpenEvent *event = static_cast<QFileOpenEvent*>(e);

		QString name = event->file();
		if(name.trimmed().isNull())
			return false;

		e->accept();

		Location *loc = new Location(name);

		gMainWindow->openSingleFile(loc);

		delete loc;

		return true;
	}

	return QApplication::event(e);
}
