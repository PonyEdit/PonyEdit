#ifndef OPTIONS_H
#define OPTIONS_H

#include <QFont>

class Options
{
public:
	enum IndentModes
	{
		NoAutoIndent = 0,
		KeepIndentOnNextLine = 1,
		SmartIndent = 2  // Not used yet; reserved.
	};

	static void save();
	static void load();

	static QFont EditorFont;
	static bool WordWrap;
	static int TabStopWidth;
	static IndentModes IndentMode;
};

#endif // OPTIONS_H
