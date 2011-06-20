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

void SyntaxHighlighter::highlightBlock(const QString &fullText)
{
	QStack<ContextDefLink> contextStack;

	//	Only highlight up to the first MAX_HIGHLIGHT_LENGTH characters.
	QString truncated;
	bool isTruncated = fullText.length() > MAX_HIGHLIGHT_LENGTH;
	if (isTruncated)
		truncated = fullText.mid(0, MAX_HIGHLIGHT_LENGTH);
	const QString& text = (isTruncated ? truncated : fullText);

	//	Get a copy of the context stack leftover from the last block
	QTextBlock previousBlock = currentBlock().previous();
	SyntaxBlockData* previousBlockData = (previousBlock.isValid() ? static_cast<SyntaxBlockData*>(previousBlock.userData()) : NULL);
	if (previousBlockData)
		contextStack = previousBlockData->mStack;

	int position = 0;
	bool firstIteration = true;
	const SyntaxDefinition::ContextLink* lineEndOverrideContextLink = NULL;
	while (position < text.length())
	{
		//	If there is no current context, create a default one
		if (contextStack.isEmpty())
			contextStack.push(mSyntaxDefinition->getDefaultContext());

		//	Take the topmost context
		ContextDefLink context = contextStack.top();

		//	Change contexts for lineBegin.
		if (firstIteration)
		{
			applyContextLink(&context->lineBeginContextLink, &contextStack, NULL);
			firstIteration = false;
			continue;
		}

		//	Cycle through all the rules in the context, looking for a match...
		int matchLength = 0;
		SyntaxDefinition::ItemData* attributeLink = NULL;
		const SyntaxDefinition::ContextLink* contextLink = NULL;
		bool isLookAhead = false;
		QStringList dynamicCaptures;
		foreach (QSharedPointer<SyntaxRule> rule, context->rules)
		{
			//	For all other (normal) rules, look for a match.
			matchLength = rule->match(text, position);
			if (matchLength > 0)
			{
				//	Match! Take note of the attribute and context links
				attributeLink = rule->getAttributeLink();
				contextLink = &rule->getContextLink();
				isLookAhead = rule->isLookAhead();
				dynamicCaptures = rule->getDynamicCaptures();

				//	Special case: If this rule is a lineContinue, override lineEnd of the current context
				if (rule->getType() == SyntaxRule::LineContinue)
				{
					lineEndOverrideContextLink = contextLink;
					contextLink = NULL;
				}

				break;
			}
		}

		//	If no match was found, check for any fallthrough rules. Force a match length of at least 1 at this point.
		if (matchLength == 0)
		{
			if (context->fallthrough)
				contextLink = &context->fallthroughContextLink;
			else if (matchLength == 0)
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

		if (contextLink)
			applyContextLink(contextLink, &contextStack, &dynamicCaptures);

		//	Move cursor (if rule is not lookahead)
		if (!isLookAhead)
			position += matchLength;
	}

	if (isTruncated)
	{
		//	If this line is truncated, drop all context information
		contextStack.clear();
	}
	else if (contextStack.size())
	{
		//	Change contexts for line end

		ContextDefLink context = contextStack.top();
		ContextDefLink lastContext;

		if (lineEndOverrideContextLink != NULL)
			applyContextLink(lineEndOverrideContextLink, &contextStack, NULL);
		else
		{
			while (!context.isNull() && context != lastContext)
			{
				applyContextLink(&context->lineEndContextLink, &contextStack, NULL);

				lastContext = context;
				if (contextStack.size()) context = contextStack.top();
			}
		}
	}

	//	Check if this highlight block is ending on a different stack to the last time
	SyntaxBlockData* oldBlockData = static_cast<SyntaxBlockData*>(currentBlock().userData());
	bool changed = true;
	if (oldBlockData)
		changed = (contextStack != oldBlockData->mStack);
	if (changed)
	{
		setCurrentBlockUserData(new SyntaxBlockData(contextStack));
		setCurrentBlockState(currentBlockState() + 1);
	}
}

void SyntaxHighlighter::applyContextLink(const SyntaxDefinition::ContextLink* contextLink, QStack<ContextDefLink>* contextStack, QStringList* dynamicCaptures)
{
	if (contextLink->contextDef)
	{
		if (contextLink->contextDef->dynamic && dynamicCaptures && dynamicCaptures->length())
			contextStack->push(duplicateDynamicContext(contextLink->contextDef, *dynamicCaptures));
		else
			contextStack->push(contextLink->contextDef);
	}
	else for (int i = 0; i < contextLink->popCount; i++)
		contextStack->pop();
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

void SyntaxHighlighter::setSyntaxDefinition(SyntaxDefinition* definition)
{
	mSyntaxDefinition = definition;
	rehighlight();
}









