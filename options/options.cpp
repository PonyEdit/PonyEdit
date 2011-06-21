#include "options.h"
#include <QSettings>
#include "main/global.h"

QFont Options::EditorFont;
int Options::EditorFontZoom;
int Options::TabStopWidth;
bool Options::WordWrap;
Options::IndentModes Options::IndentMode;
bool Options::IndentSpaces;

Options::StartupActions Options::StartupAction;
QStringList Options::StartupFiles;
QList<int> Options::StartupFilesLineNo;
bool Options::ShutdownPrompt;

void Options::save()
{
	QSettings settings;

	settings.setValue(ntr("editorFont"), QVariant(EditorFont.toString()));
	settings.setValue(ntr("editorFontZoom"), QVariant(EditorFontZoom));
	settings.setValue(ntr("wordWrap"), QVariant(WordWrap));
	settings.setValue(ntr("tabStopWidth"), QVariant(TabStopWidth));
	settings.setValue(ntr("indentMode"), QVariant(static_cast<int>(IndentMode)));
	settings.setValue(ntr("indentSpaces"), QVariant(static_cast<int>(IndentSpaces)));

	settings.setValue(ntr("StartupAction"), QVariant(static_cast<int>(StartupAction)));
	settings.setValue(ntr("ShutdownPrompt"), QVariant(ShutdownPrompt));

	settings.beginWriteArray("StartupFiles");
	int skipped = 0;
	QString file;
	for(int ii = 0; ii + skipped < StartupFiles.length(); ii++)
	{
		file = StartupFiles[ii].trimmed();
		if(file.isNull())
		{
			skipped++;
			continue;
		}
		settings.setArrayIndex(ii - skipped);
		settings.setValue(ntr("path"), QVariant(file));
		settings.setValue(ntr("line"), QVariant(StartupFilesLineNo[ii]));
	}
	settings.endArray();
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
	TabStopWidth = settings.value(ntr("tabStopWidth"), QVariant(8)).toInt();
	IndentMode = static_cast<IndentModes>(settings.value(ntr("indentMode"), QVariant(static_cast<int>(KeepIndentOnNextLine))).toInt());
	IndentSpaces = settings.value(ntr("indentSpaces"), QVariant(false)).toBool();

	StartupAction = static_cast<StartupActions>(settings.value(ntr("StartupAction"), QVariant(static_cast<int>(NoFiles))).toInt());
	ShutdownPrompt = settings.value(ntr("ShutdownPrompt"), QVariant(true)).toBool();

	int count = settings.beginReadArray("StartupFiles");
	for (int ii = 0; ii < count; ii++)
	{
		settings.setArrayIndex(ii);

		StartupFiles.append(settings.value(ntr("path")).toString());
		StartupFilesLineNo.append(settings.value(ntr("line"), QVariant(1)).toInt());
	}
	settings.endArray();
}
