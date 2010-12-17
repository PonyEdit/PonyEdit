#include <QtGui>

#include "syntaxhighlighter.h"
#include "syntaxdefinition.h"
#include "syntaxrule.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent, SyntaxDefinition* syntaxDef)
	: QSyntaxHighlighter(parent)
{
	mSyntaxDefinition = syntaxDef;

	mDefaultColors.insert("dsnormal", QColor("black"));
	mDefaultColors.insert("dskeyword", QColor("blue"));
	mDefaultColors.insert("dsfunction", QColor("cornflowerblue"));
	mDefaultColors.insert("dsdatatype", QColor("magenta"));
	mDefaultColors.insert("dsdecval", QColor("red"));
	mDefaultColors.insert("dsbasen", QColor("red"));
	mDefaultColors.insert("dsfloat", QColor("red"));
	mDefaultColors.insert("dsstring", QColor("red"));
	mDefaultColors.insert("dschar", QColor("red"));
	mDefaultColors.insert("dscomment", QColor("green"));
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
	//	Work out what the current context is
	int prevState = previousBlockState();
	SyntaxDefinition::Context* context = (prevState < 0 ? mSyntaxDefinition->getDefaultContext() : mSyntaxDefinition->getContextByIndex(prevState));

	int position = 0;
	while (position < text.length())
	{
		//	Cycle through all the rules in the context, looking for a match...
		foreach (SyntaxRule* rule, context->rules)
		{
			int matchLength = rule->match(text, position);
			if (matchLength > 0)
			{
				QColor color;

				SyntaxDefinition::ItemData* id = rule->getAttributeLink();
				if (id)
					color = mDefaultColors.value(id->styleName.toLower());
				else
					color = QColor("lightslategray");

				setFormat(position, matchLength, color);
				position += matchLength - 1;
				break;
			}
		}

		position++;
	}
}
