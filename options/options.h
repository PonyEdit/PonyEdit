#ifndef OPTIONS_H
#define OPTIONS_H

#include <QFont>

class Options
{
public:
	static inline QFont& getEditorFont() { return mEditorFont; }
	static inline void setEditorFont(QFont font) { mEditorFont = font; }

	static void save();
	static void load();

private:
	static QFont mEditorFont;
};

#endif // OPTIONS_H
