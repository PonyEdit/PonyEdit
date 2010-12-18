#include <QtGui>

#include "syntaxhighlighter.h"
#include "syntaxdefinition.h"
#include "syntaxrule.h"
#include "syntaxblockdata.h"

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
	QStack<Context> contextStack;

	/*
		//	Get a copy of the context stack leftover from the last block
		QTextBlock previousBlock = currentBlock().previous();
		SyntaxBlockData* previousBlockData = (previousBlock.isValid() ? static_cast<SyntaxBlockData*>(previousBlock.userData()) : NULL);
	*/

	int position = 0;
	while (position < text.length())
	{
		//	If there is no current context, create a default one
		if (contextStack.isEmpty())
		{
			Context defaultContext;
			defaultContext.definition = mSyntaxDefinition->getDefaultContext();
			contextStack.push(defaultContext);
		}

		//	Take the topmost context
		Context* context = &contextStack.top();
		SyntaxDefinition::ContextDef* contextDef = context->definition;

		//	Cycle through all the rules in the context, looking for a match...
		foreach (SyntaxRule* rule, contextDef->rules)
		{
			int matchLength = rule->match(text, position);
			if (matchLength > 0)
			{
				//	Match! Apply a colour to the text...
				SyntaxDefinition::ItemData* id = rule->getAttributeLink();
				QColor color = id ? mDefaultColors.value(id->styleName.toLower()) : QColor("lightslategray");
				setFormat(position, matchLength, color);

				//	Change context (if the rule specifies one)
				if (rule->getContextLink())
				{
					Context newContext;
					newContext.definition = rule->getContextLink();
					contextStack.push(newContext);
				}

				//	Pop contexts (if the rule says to)
				for (int i = 0; i < rule->getPopCount(); i++)
					contextStack.pop();

				//	Move cursor (if rule is not lookahead)
				if (!rule->isLookAhead())
					position += matchLength;
				position--;
				break;
			}
		}

		position++;
	}
}
