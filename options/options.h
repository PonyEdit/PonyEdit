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

	enum FileListTypes
	{
		QuickList = 0,
		TabbedList = 1
	};

	static void save();
	static void load();

	static QFont* EditorFont;
	static int EditorFontZoom;
	static bool WordWrap;
	static int TabStopWidth;
	static IndentModes IndentMode;
	static bool IndentSpaces;		//	Indent with spaces instead of tabs?
	static bool StripSpaces;

	static StartupActions StartupAction;
	static QStringList StartupFiles;
	static QList<int> StartupFilesLineNo;

	static bool ShutdownPrompt;

	static FileListTypes FileListType;

	static int LoggingLevel;
};

#endif // OPTIONS_H
