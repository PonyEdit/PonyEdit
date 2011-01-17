#include "options.h"
#include <QSettings>
#include "main/global.h"

QFont Options::EditorFont;
int Options::TabStopWidth;
bool Options::WordWrap;

void Options::save()
{
	QSettings settings;

	settings.setValue(ntr("editorFont"), QVariant(EditorFont.toString()));
	settings.setValue(ntr("wordWrap"), QVariant(WordWrap));
	settings.setValue(ntr("tabStopWidth"), QVariant(TabStopWidth));
}

void Options::load()
{
	QSettings settings;

	QString fontString = settings.value(ntr("editorFont")).toString();
	EditorFont.fromString(fontString);
	if (EditorFont.family().isEmpty())
		EditorFont.setFamily(ntr("inconsolata"));

	WordWrap = settings.value(ntr("wordWrap"), QVariant(false)).toBool();
	TabStopWidth = settings.value(ntr("TabStopWidth"), QVariant(8)).toInt();
}
