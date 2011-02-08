#ifndef OPTIONS_H
#define OPTIONS_H

#include <QFont>
#include <QStringList>
#include <QList>

class Options
{
public:
	enum IndentModes
	{
		NoAutoIndent = 0,
		KeepIndentOnNextLine = 1,
		SmartIndent = 2  // Not used yet; reserved.
	};

	enum StartupActions
	{
		NoFiles = 0,
		BlankFile = 1,
		ReopenFiles = 2,
		SetFiles = 3
	};

	static void save();
	static void load();

	static QFont EditorFont;
	static int EditorFontZoom;
	static bool WordWrap;
	static int TabStopWidth;
	static IndentModes IndentMode;

	static StartupActions StartupAction;
	static QStringList StartupFiles;
	static QList<int> StartupFilesLineNo;

	static bool ShutdownPrompt;
};

#endif // OPTIONS_H
