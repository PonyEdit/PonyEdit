#ifndef OPTIONS_H
#define OPTIONS_H

#include <QFont>

class Options
{
public:
	static void save();
	static void load();

	static QFont EditorFont;
	static bool WordWrap;
	static int TabStopWidth;
};

#endif // OPTIONS_H
