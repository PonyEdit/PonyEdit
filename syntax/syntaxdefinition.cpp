#include <QtXml>
#include <QFile>
#include "main/tools.h"
#include "syntaxrule.h"
#include "syntaxdefinition.h"
#include "syntaxdefxmlhandler.h"

SyntaxDefinition::ContextDef::ContextDef() : fallthrough(false), dynamic(false), listIndex(0), attributeLink(NULL) {};
SyntaxDefinition::ContextDef::~ContextDef() {}

SyntaxDefinition::SyntaxDefinition(const QString& filename)
{
	mValid = false;
	mDeliminators = ".():!+,-<=>%&/;?[]^{|}~\\*, \t";

	QFile file(filename);
	if (file.open(QFile::ReadOnly))
	{
		SyntaxDefXmlHandler handler(this);
		QXmlSimpleReader reader;
		reader.setContentHandler(&handler);
		reader.setErrorHandler(&handler);

		if (reader.parse(&file))
			if (link())
				mValid = true;
	}

	if (!mValid)
		qDebug() << "Failed to read syntax definition";
}

bool SyntaxDefinition::link()
{
	//	Go through the rules in all contexts
	foreach (QSharedPointer<ContextDef> context, mContextList)
	{
		//	Link up this context's fallthough, lineEnd, lineBegin references (if there is one)
		if (context->fallthrough)
			linkContext(context->fallthroughContext, &context->fallthroughContextLink);
		if (!context->lineBeginContext.isEmpty())
			linkContext(context->lineBeginContext, &context->lineBeginContextLink);
		if (!context->lineEndContext.isEmpty())
			linkContext(context->lineEndContext, &context->lineEndContextLink);

		//	Link up this context's attribute property (if there is one)
		if (!context->attribute.isEmpty())
			context->attributeLink = getItemData(context->attribute);

		//	Pick through the rules in this context, linking <IncludeRules>, context="", etc.
		for (int i = 0; i < context->rules.length(); i++)
		{
			QSharedPointer<SyntaxRule> rule = context->rules[i];

			//	Deal with <IncludeRules> tags
			if (rule->getType() == SyntaxRule::IncludeRules)
			{
				//	Remove the tag...
				context->rules.removeAt(i);

				//	Parse it.
				if (rule->getIncludeAttrib())
					qDebug() << "Warning: Include attrib enabled on includerule block; ignored";

				QString source = rule->getContext();
				QSharedPointer<ContextDef> sourceContext;
				if (source.startsWith("##"))
				{
					SyntaxDefinition* includedDefinition = gSyntaxDefManager.getDefinitionForSyntax(source.mid(2));
					if (includedDefinition)
						sourceContext = includedDefinition->getDefaultContext();
				}
				else
					sourceContext = getContext(source);

				if (sourceContext.isNull())
					qDebug() << "Warning: IncludeRule names non-existent context: " << source;
				else
				{
					if (rule->getIncludeAttrib())
					{
						context->attribute = sourceContext->attribute;
						context->attributeLink = sourceContext->attributeLink;
					}

					int insertionOffset = 0;
					foreach (const QSharedPointer<SyntaxRule>& copyRule, sourceContext->rules)
						context->rules.insert(i + insertionOffset++, copyRule);
				}

				i--;
			}
			else if (!rule->link(this))
				return false;
		}
	}

	return true;
}

bool SyntaxDefinition::linkContext(const QString& context, ContextLink* link)
{
	if (context.startsWith('#') || context.isEmpty())
	{
		if (context.startsWith("#pop", Qt::CaseInsensitive))
			link->popCount = context.count('#');
	}
	else
	{
		link->contextDef = getContext(context);
		if (!link->contextDef)
		{
			qDebug() << "Failed to link context: " << context;
			return false;
		}
	}

	return true;
}

void SyntaxDefinition::addKeywordList(KeywordList* list)
{
	mKeywordLists.insert(list->name.toLower(), list);
}

void SyntaxDefinition::addContext(ContextDef* context)
{
	QSharedPointer<ContextDef> wrappedContext = QSharedPointer<ContextDef>(context);

	if (mDefaultContext.isNull())
		mDefaultContext = wrappedContext;
	mContextMap.insert(context->name.toLower(), wrappedContext);
	mContextList.append(wrappedContext);
}

void SyntaxDefinition::addItemData(ItemData* itemData)
{
	mItemDatas.insert(itemData->name.toLower(), itemData);
}









