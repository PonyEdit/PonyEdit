#include <QtXml>
#include <QFile>
#include "main/tools.h"
#include "syntax/syntaxrule.h"
#include "syntax/syntaxdefinition.h"
#include "syntax/syntaxdefxmlhandler.h"

SyntaxDefinition* gTestSyntaxDef = new SyntaxDefinition("syntaxdefs/perl.xml");

SyntaxDefinition::SyntaxDefinition(const QString& filename)
{
	mValid = false;
	mDefaultContext = NULL;

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
	//	Go through the rules in all contexts, linking <IncludeRules>, context="", etc.
	foreach (Context* context, mContextList)
	{
		for (int i = 0; i < context->rules.length(); i++)
		{
			SyntaxRule* rule = context->rules[i];

			//	Deal with <IncludeRules> tags
			if (rule->getType() == SyntaxRule::IncludeRules)
			{
				//	Remove the tag...
				context->rules.removeAt(i);

				//	Parse it.
				if (rule->getIncludeAttrib())
					qDebug() << "Warning: Include attrib enabled on includerule block; ignored";

				QString source = rule->getContext();
				if (source.startsWith("##"))
					qDebug() << "Warning: Include from another file; ignored";
				else
				{
					Context* otherContext = getContext(source);
					if (!otherContext)
						qDebug() << "Warning: IncludeRule names non-existent context: " << source;
					else
					{
						//	Copy all the rules from the other context to this one
						foreach (SyntaxRule* copyRule, otherContext->rules)
							context->rules.insert(i, new SyntaxRule(*copyRule));
					}
				}

				i--;
			}
			else if (!rule->link(this))
				return false;
		}
	}

	return true;
}

void SyntaxDefinition::addKeywordList(KeywordList* list)
{
	mKeywordLists.insert(list->name.toLower(), list);
}

void SyntaxDefinition::addContext(Context* context)
{
	if (mDefaultContext == NULL)
		mDefaultContext = context;
	mContextMap.insert(context->name.toLower(), context);
	mContextList.append(context);
}

void SyntaxDefinition::addItemData(ItemData* itemData)
{
	mItemDatas.insert(itemData->name.toLower(), itemData);
}











