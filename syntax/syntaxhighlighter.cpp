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
	mDefaultColors.insert("dskeyword", QColor("steelblue"));
	mDefaultColors.insert("dsdatatype", QColor("dodgerblue"));
	mDefaultColors.insert("dsdecval", QColor("firebrick"));
	mDefaultColors.insert("dsbasen", QColor("firebrick"));
	mDefaultColors.insert("dsfloat", QColor("firebrick"));
	mDefaultColors.insert("dschar", QColor("firebrick"));
	mDefaultColors.insert("dsstring", QColor("firebrick"));
	mDefaultColors.insert("dscomment", QColor("limegreen"));
	mDefaultColors.insert("dsothers", QColor("purple"));
	mDefaultColors.insert("dsalert", QColor("red"));
	mDefaultColors.insert("dsfunction", QColor("purple"));
	mDefaultColors.insert("dsregionmarker", QColor("chocolate"));
	mDefaultColors.insert("dserror", QColor("red"));
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
		bool matchFound = false;
		foreach (SyntaxRule* rule, contextDef->rules)
		{
			int matchLength = rule->match(text, position);
			if (matchLength > 0)
			{
				//	Match! Apply a colour to the text...
				SyntaxDefinition::ItemData* id = rule->getAttributeLink();

				QColor color = id ? mDefaultColors.value(id->styleName.toLower()) : QColor("orange");
				setFormat(position, matchLength, color);

				//	Change context (if the rules say to)
				applyContextLink(rule->getContextLink(), &contextStack);

				//	Move cursor (if rule is not lookahead)
				if (!rule->isLookAhead())
					position += matchLength;

				matchFound = true;
				break;
			}
		}

		//	If no match was found, check for a fallthrough context. If none specified, just move over and try again :-/
		if (!matchFound)
		{
			if (contextDef->fallthrough)
				applyContextLink(contextDef->fallthroughContextLink, &contextStack);
			else
			{
				if (contextDef->attributeLink != NULL)
				{
					if (!mDefaultColors.contains(contextDef->attributeLink->styleName.toLower()))
						qDebug() << "No style name: " << contextDef->attributeLink->styleName;
					setFormat(position, 1, mDefaultColors.value(contextDef->attributeLink->styleName.toLower()));
				}
				else if (contextStack.size() > 1)
					qDebug() << "No Attribute :(";
				position++;
			}
		}
	}
}

void SyntaxHighlighter::applyContextLink(const SyntaxDefinition::ContextLink& link, QStack<Context>* contextStack)
{
	if (link.contextDef)
	{
		Context newContext;
		newContext.definition = link.contextDef;
		contextStack->push(newContext);
	}
	else for (int i = 0; i < link.popCount; i++)
		contextStack->pop();
}



