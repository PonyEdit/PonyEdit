#include "options.h"
#include <QSettings>
#include "main/global.h"

QFont Options::EditorFont;
int Options::EditorFontZoom;
int Options::TabStopWidth;
bool Options::WordWrap;
Options::IndentModes Options::IndentMode;

void Options::save()
{
	QSettings settings;

	settings.setValue(ntr("editorFont"), QVariant(EditorFont.toString()));
	settings.setValue(ntr("editorFontZoom"), QVariant(EditorFontZoom));
	settings.setValue(ntr("wordWrap"), QVariant(WordWrap));
	settings.setValue(ntr("tabStopWidth"), QVariant(TabStopWidth));
	settings.setValue(ntr("indentMode"), QVariant(static_cast<int>(IndentMode)));
}

void Options::load()
{
	QSettings settings;

	QString fontString = settings.value(ntr("editorFont")).toString();
	EditorFont = QFont();
	EditorFont.fromString(fontString);
	if (EditorFont.family().isEmpty())
		EditorFont.setFamily(ntr("inconsolata"));

	EditorFontZoom = settings.value(ntr("editorFontZoom"), QVariant(100)).toInt();
	WordWrap = settings.value(ntr("wordWrap"), QVariant(false)).toBool();
	TabStopWidth = settings.value(ntr("TabStopWidth"), QVariant(8)).toInt();
	IndentMode = static_cast<IndentModes>(settings.value(ntr("indentMode"), QVariant(static_cast<int>(KeepIndentOnNextLine))).toInt());
}
