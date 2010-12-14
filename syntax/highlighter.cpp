#include <QtGui>

#include "syntaxhighlighter.h"
#include "syntaxdefinition.h"
#include "syntaxrule.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent, SyntaxDefinition* syntaxDef)
	: QSyntaxHighlighter(parent)
{
	mSyntaxDefinition = syntaxDef;
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
				setFormat(position, matchLength, QColor(Qt::red));
				position += matchLength - 1;
				break;
			}
		}

		position++;
	}
}
