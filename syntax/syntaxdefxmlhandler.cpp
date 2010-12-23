#include "syntaxdefxmlhandler.h"
#include "syntaxdefinition.h"
#include "syntaxrule.h"
#include "main/tools.h"

SyntaxDefXmlHandler::SyntaxDefXmlHandler(SyntaxDefinition* definition)
{
	mRecord = NULL;
	mDefinition = definition;
	mCurrentBlocks = None;

	mKeywordList = NULL;
	mContext = NULL;
	mRule = NULL;
}

SyntaxDefXmlHandler::SyntaxDefXmlHandler(SyntaxDefManager::Record* record)
{
	mRecord = record;
	mDefinition = NULL;
	mCurrentBlocks = None;
}

QString SyntaxDefXmlHandler::errorString() const { return QString(); }
bool SyntaxDefXmlHandler::startElement(const QString &/* namespaceURI */, const QString &localName, const QString &/* qName */, const QXmlAttributes &atts)
{
	switch (mCurrentBlocks)
	{
		case None:
			//	Not inside any blocks, look for a <language> block.
			if (localName.compare("language", Qt::CaseInsensitive) == 0)
			{
				//	If fetching a Manager record only, only care about the language block
				if (mRecord)
				{
					mRecord->pack(atts);
					return false;
				}

				mCurrentBlocks |= Language;
			}
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
				mContext = new SyntaxDefinition::ContextDef();
				mContext->name = Tools::getStringXmlAttribute(atts, "name");
				mContext->attribute = Tools::getStringXmlAttribute(atts, "attribute");
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
					mContext->rules.append(QSharedPointer<SyntaxRule>(rule));
				else
					mRule->addChildRule(QSharedPointer<SyntaxRule>(rule));

				mRule = rule;
				mCurrentBlocks |= Rule;
			}
			else
				delete rule;
			break;
		}

		case Language|Highlighting|ItemDatas:
			//	Inside an <ItemDatas> block. Just expect a bunch of <itemdata> entries
			if (localName.compare("itemdata", Qt::CaseInsensitive) == 0)
			{
				SyntaxDefinition::ItemData* itemData = new SyntaxDefinition::ItemData();
				itemData->name = Tools::getStringXmlAttribute(atts, "name");
				itemData->styleName = Tools::getStringXmlAttribute(atts, "defStyleNum");
				itemData->color = Tools::getStringXmlAttribute(atts, "color");
				itemData->selColor = Tools::getStringXmlAttribute(atts, "selColor");
				itemData->italic = Tools::getIntXmlAttribute(atts, "italic", 0);
				itemData->bold = Tools::getIntXmlAttribute(atts, "bold", 0);
				itemData->underline = Tools::getIntXmlAttribute(atts, "underline", 0);
				itemData->strikeout = Tools::getIntXmlAttribute(atts, "strikeout", 0);
				mDefinition->addItemData(itemData);
			}
			break;

		case Language|General:
			if (localName.compare("comments", Qt::CaseInsensitive) == 0)
				mCurrentBlocks |= Comments;
			else if (localName.compare("folding", Qt::CaseInsensitive) == 0)
				mDefinition->setIndentationSensitive(Tools::getIntXmlAttribute(atts, "indentationsensitive", 0));
			else if (localName.compare("keywords", Qt::CaseInsensitive) == 0)
			{
				mDefinition->setCaseSensitiveKeywords(Tools::getIntXmlAttribute(atts, "casesensitive", 1));
				mDefinition->setWeakDeliminators(Tools::getStringXmlAttribute(atts, "weakdeliminator"));
				mDefinition->setAdditionalDeliminators(Tools::getStringXmlAttribute(atts, "additionaldeliminators"));
				mDefinition->setWordWrapDeliminator(Tools::getStringXmlAttribute(atts, "wordwrapdeliminator"));
			}
			break;

		case Language|General|Comments:
			if (localName.compare("comment", Qt::CaseInsensitive) == 0)
			{
				SyntaxDefinition::CommentStyle* style = new SyntaxDefinition::CommentStyle();
				style->multiline = (Tools::getStringXmlAttribute(atts, "name").compare("multiline", Qt::CaseInsensitive) == 0);
				style->start = Tools::getStringXmlAttribute(atts, "start");
				style->end = Tools::getStringXmlAttribute(atts, "end");
			}
			break;
	}

	return true;
}

bool SyntaxDefXmlHandler::endElement(const QString &/* namespaceURI */, const QString &localName, const QString &/* qName */)
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

		case Language|Highlighting|ItemDatas:
			if (localName.compare("itemdatas", Qt::CaseInsensitive) == 0)
				mCurrentBlocks &= ~ItemDatas;
			break;

		case Language|General:
			if (localName.compare("general", Qt::CaseInsensitive) == 0)
				mCurrentBlocks &= ~General;
			break;

		case Language|General|Comments:
			if (localName.compare("comments", Qt::CaseInsensitive) == 0)
				mCurrentBlocks &= ~Comments;
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
		{
			//	Keep KeywordLists sorted in reverse-length order
			int i;
			for (i = 0; i < mKeywordList->items.length(); i++)
				if (mKeywordList->items[i].length() < trimmed.length())
					break;
			mKeywordList->items.insert(i, trimmed);
		}
	}

	return true;
}






















