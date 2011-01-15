#include "options.h"
#include <QSettings>
#include "main/global.h"

QFont Options::mEditorFont;

void Options::save()
{
	QSettings settings;

	settings.setValue(ntr("editorFont"), QVariant(mEditorFont.toString()));
}

void Options::load()
{
	QSettings settings;

	QString fontString = settings.value(ntr("editorFont")).toString();
	mEditorFont.fromString(fontString);
	if (mEditorFont.family().isEmpty())
		mEditorFont.setFamily(ntr("inconsolata"));
}
