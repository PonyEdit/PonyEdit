#include "syntaxdefxmlhandler.h"
#include "syntaxdefinition.h"
#include "syntaxrule.h"
#include "main/tools.h"

SyntaxDefXmlHandler::SyntaxDefXmlHandler(SyntaxDefinition* definition)
{
	mDefinition = definition;
	mCurrentBlocks = None;

	mKeywordList = NULL;
	mContext = NULL;
	mRule = NULL;
}

bool SyntaxDefXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
	switch (mCurrentBlocks)
	{
		case None:
			//	Not inside any blocks, look for a <language> block.
			if (localName.compare("language", Qt::CaseInsensitive) == 0)
				mCurrentBlocks |= Language;
			break;

		case Language:
			//	Inside <language>. Look for <highlighting> and <general>
			if (localName.compare("highlighting", Qt::CaseInsensitive) == 0)
				mCurrentBlocks |= Highlighting;
			else if (localName.compare("general", Qt::CaseInsensitive) == 0)
				mCurrentBlocks |= General;
			break;

		case Language|Highlighting:
			//	Inside <highlighting>. Look for <list>, <contexts> and <itemDatas>
			if (localName.compare("list", Qt::CaseInsensitive) == 0)
			{
				mCurrentBlocks |= List;
				mKeywordList = new SyntaxDefinition::KeywordList();
				mKeywordList->name = Tools::getStringXmlAttribute(atts, "name");
				mDefinition->addKeywordList(mKeywordList);
			}
			else if (localName.compare("contexts", Qt::CaseInsensitive) == 0)
				mCurrentBlocks |= Contexts;
			else if (localName.compare("itemdatas", Qt::CaseInsensitive) == 0)
				mCurrentBlocks |= ItemDatas;
			break;

		case Language|Highlighting|List:
			//	Inside <list>. Only care about <item> entries
			if (localName.compare("item", Qt::CaseInsensitive) == 0)
				mCurrentBlocks |= Item;
			break;

		case Language|Highlighting|Contexts:
			//	Inside <contexts>. Only care about <context> entries
			if (localName.compare("context", Qt::CaseInsensitive) == 0)
			{
				mCurrentBlocks |= Context;
				mContext = new SyntaxDefinition::Context();
				mContext->name = Tools::getStringXmlAttribute(atts, "name");
				mContext->lineEndContext = Tools::getStringXmlAttribute(atts, "lineendcontext");
				mContext->lineBeginContext = Tools::getStringXmlAttribute(atts, "linebegincontext");
				mContext->fallthrough = Tools::getIntXmlAttribute(atts, "fallthrough", 0);
				mContext->fallthroughContext = Tools::getStringXmlAttribute(atts, "fallthroughcontext");
				mContext->dynamic = Tools::getIntXmlAttribute(atts, "dynamic", 0);
				mDefinition->addContext(mContext);
			}
			break;

		case Language|Highlighting|Contexts|Context:
		case Language|Highlighting|Contexts|Context|Rule:
		{
			//	Inside a <context> or a <rule>. Should be a big pile of rules.
			SyntaxRule* rule = new SyntaxRule(mRule, localName, atts);
			if (rule->isValid())
			{
				if (mRule == NULL)
					mDefinition->addRule(rule);
				else
					mRule->addChildRule(rule);

				qDebug() << "Added rule: " << rule->getName() << " Is child = " << (mRule != NULL);

				mRule = rule;
				mCurrentBlocks |= Rule;
			}
			else
				delete rule;
			break;
		}
	}

	return true;
}

bool SyntaxDefXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	switch (mCurrentBlocks)
	{
		case Language:
			if (localName.compare("language", Qt::CaseInsensitive) == 0)
				mCurrentBlocks = None;
			break;

		case Language|Highlighting:
			if (localName.compare("highlighting", Qt::CaseInsensitive) == 0)
				mCurrentBlocks &= ~Highlighting;
			break;

		case Language|Highlighting|List:
			if (localName.compare("list", Qt::CaseInsensitive) == 0)
				mCurrentBlocks &= ~List;
			break;

		case Language|Highlighting|List|Item:
			if (localName.compare("item", Qt::CaseInsensitive) == 0)
				mCurrentBlocks &= ~Item;
			break;

		case Language|Highlighting|Contexts:
			if (localName.compare("contexts", Qt::CaseInsensitive) == 0)
				mCurrentBlocks &= ~Contexts;
			break;

		case Language|Highlighting|Contexts|Context:
			if (localName.compare("context", Qt::CaseInsensitive) == 0)
				mCurrentBlocks &= ~Context;
			break;

		case Language|Highlighting|Contexts|Context|Rule:
			if (localName.compare(mRule->getName(), Qt::CaseInsensitive) == 0)
			{
				mRule = mRule->getParent();
				if (mRule == NULL)
					mCurrentBlocks &= ~Rule;
			}
			break;
	}

	return true;
}

bool SyntaxDefXmlHandler::characters(const QString &ch)
{
	if (mCurrentBlocks == (Language|Highlighting|List|Item))
	{
		QString trimmed = ch.trimmed();
		if (!trimmed.isEmpty())
			mKeywordList->items.append(trimmed);
	}

	return true;
}






















