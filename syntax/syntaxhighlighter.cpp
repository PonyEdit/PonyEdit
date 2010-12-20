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
		int matchLength = 0;
		SyntaxDefinition::ItemData* attributeLink = NULL;
		const SyntaxDefinition::ContextLink* contextLink = NULL;
		bool isLookAhead = false;
		foreach (SyntaxRule* rule, contextDef->rules)
		{
			matchLength = rule->match(text, position);
			if (matchLength > 0)
			{
				//	Match! Take note of the attribute and context links
				attributeLink = rule->getAttributeLink();
				contextLink = &rule->getContextLink();
				isLookAhead = rule->isLookAhead();
				break;
			}
		}

		//	If no match was found, check for any fallthrough rules. Force a match length of at least 1 at this point.
		if (matchLength == 0)
		{
			if (contextDef->fallthrough)
				contextLink = &contextDef->fallthroughContextLink;
			matchLength = 1;
		}

		//	If no attribute link was found, search through the context stack for one
		if (!attributeLink)
		{
			QStack<Context>::iterator it = contextStack.end();
			while (it != contextStack.begin())
			{
				--it;
				const Context& scanContext = *it;
				if ((attributeLink = scanContext.definition->attributeLink) != NULL)
					break;
			}
		}

		//	If an attribute link was found, apply it to the text
		if (attributeLink)
		{
			if (mDefaultColors.contains(attributeLink->styleName.toLower()))
				setFormat(position, matchLength, mDefaultColors.value(attributeLink->styleName.toLower()));
			else
				qDebug() << "No style name: " << attributeLink->styleName;
		}

		//	If there is a context link (either from a rule, or from a fallthrough context)
		if (contextLink)
			applyContextLink(contextLink, &contextStack);

		//	Move cursor (if rule is not lookahead)
		if (!isLookAhead)
			position += matchLength;
	}
}

void SyntaxHighlighter::applyContextLink(const SyntaxDefinition::ContextLink* link, QStack<Context>* contextStack)
{
	if (link->contextDef)
	{
		Context newContext;
		newContext.definition = link->contextDef;
		contextStack->push(newContext);
	}
	else for (int i = 0; i < link->popCount; i++)
		contextStack->pop();
}



