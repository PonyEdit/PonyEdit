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
	QStack<ContextDefLink> contextStack;

	//	Get a copy of the context stack leftover from the last block
	QTextBlock previousBlock = currentBlock().previous();
	SyntaxBlockData* previousBlockData = (previousBlock.isValid() ? static_cast<SyntaxBlockData*>(previousBlock.userData()) : NULL);
	if (previousBlockData)
		contextStack = previousBlockData->mStack;

	int position = 0;
	while (position < text.length())
	{
		//	If there is no current context, create a default one
		if (contextStack.isEmpty())
			contextStack.push(mSyntaxDefinition->getDefaultContext());

		//	Take the topmost context
		ContextDefLink context = contextStack.top();

		//	Cycle through all the rules in the context, looking for a match...
		int matchLength = 0;
		SyntaxDefinition::ItemData* attributeLink = NULL;
		const SyntaxDefinition::ContextLink* contextLink = NULL;
		bool isLookAhead = false;
		QStringList dynamicCaptures;
		foreach (QSharedPointer<SyntaxRule> rule, context->rules)
		{
			matchLength = rule->match(text, position);
			if (matchLength > 0)
			{
				//	Match! Take note of the attribute and context links
				attributeLink = rule->getAttributeLink();
				contextLink = &rule->getContextLink();
				isLookAhead = rule->isLookAhead();
				dynamicCaptures = rule->getDynamicCaptures();
				break;
			}
		}

		//	If no match was found, check for any fallthrough rules. Force a match length of at least 1 at this point.
		if (matchLength == 0)
		{
			if (context->fallthrough)
				contextLink = &context->fallthroughContextLink;
			else
				matchLength = 1;
		}

		//	If no attribute link was found, search through the context stack for one
		if (!attributeLink && matchLength)
		{
			QStack<ContextDefLink>::iterator it = contextStack.end();
			while (it != contextStack.begin())
			{
				--it;
				const ContextDefLink& scanContext = *it;
				if ((attributeLink = scanContext->attributeLink) != NULL)
					break;
			}
		}

		//	If an attribute link was found, apply it to the text
		if (attributeLink && matchLength)
		{
			if (mDefaultColors.contains(attributeLink->styleName.toLower()))
				setFormat(position, matchLength, mDefaultColors.value(attributeLink->styleName.toLower()));
			else
				qDebug() << "No style name: " << attributeLink->styleName;
		}

		//	If there is a context link (either from a rule, or from a fallthrough context)
		if (contextLink)
		{
			if (contextLink->contextDef)
			{
				if (contextLink->contextDef->dynamic && dynamicCaptures.length())
					contextStack.push(duplicateDynamicContext(contextLink->contextDef, dynamicCaptures));
				else
					contextStack.push(contextLink->contextDef);
			}
			else for (int i = 0; i < contextLink->popCount; i++)
				contextStack.pop();
		}

		//	Move cursor (if rule is not lookahead)
		if (!isLookAhead)
			position += matchLength;
	}

	currentBlock().setUserData(new SyntaxBlockData(contextStack));
}

ContextDefLink SyntaxHighlighter::duplicateDynamicContext(const ContextDefLink& source, const QStringList& captures) const
{
	SyntaxDefinition::ContextDef* newContext = new SyntaxDefinition::ContextDef(*source.data());
	replaceDynamicRules(NULL, &newContext->rules, captures);
	return ContextDefLink(newContext);
}

void SyntaxHighlighter::replaceDynamicRules(SyntaxRule* parent, QList<QSharedPointer<SyntaxRule> >* ruleList, const QStringList& captures) const
{
	for (int i = 0; i < ruleList->length(); i++)
	{
		QSharedPointer<SyntaxRule> rule = ruleList->at(i);

		if (rule->isDynamic())
		{
			SyntaxRule* newRule = new SyntaxRule(parent, rule, false, true);
			newRule->applyDynamicCaptures(captures);
			rule = QSharedPointer<SyntaxRule>(newRule);
			ruleList->replace(i, rule);
		}

		replaceDynamicRules(rule.data(), rule->getChildRules(), captures);
	}
}











