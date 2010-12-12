#include "syntaxdefinition.h"
#include <QtXml>
#include <QFile>
#include "tools.h"
#include "syntaxrule.h"

SyntaxDefinition* gTestSyntaxDef = new SyntaxDefinition("syntaxdefs/perl.xml");

SyntaxDefinition::SyntaxDefinition(const QString& filename)
{
	mValid = false;
	mDefaultContext = NULL;

	QFile file(filename);
	if (file.open(QFile::ReadOnly))
	{
		QDomDocument document("Import");
		if (document.setContent(&file) && readXml(&document))
			mValid = true;
		file.close();
	}
}

bool SyntaxDefinition::readXml(QDomDocument* document)
{
	//	Get the top level <language> node and pick apart its attributes
	QDomElement languageNode = document->firstChildElement("language");
	if (languageNode.isNull()) return false;

	mName = Tools::getStringXmlAttribute(&languageNode, "name");
	mSection = Tools::getStringXmlAttribute(&languageNode, "section");
	mExtensions = Tools::getStringXmlAttribute(&languageNode, "extensions").split(';');
	if (mName.isNull() || mSection.isNull() || mExtensions.isEmpty()) return false;

	//	Read the highlighting block
	QDomElement highlightingNode = languageNode.firstChildElement("highlighting");
	if (highlightingNode.isNull()) return false;
	readHighlightingNode(highlightingNode);

	//	Set the defaults for general settings...
	mCaseSensitiveKeywords = true;
	mIndentationSensitive = false;

	//	Pull in any specified general settings
	QDomElement generalNode = document->firstChildElement("general");
	if (!generalNode.isNull())
		readGeneralNode(generalNode);

	return true;
}

void SyntaxDefinition::readContext(const QDomElement& contextNode)
{
	Context* context = new Context();
	context->attribute = Tools::getStringXmlAttribute(&contextNode, "attribute");
	context->name = Tools::getStringXmlAttribute(&contextNode, "name");
	context->lineBeginContext = Tools::getStringXmlAttribute(&contextNode, "linebegincontext");
	context->lineEndContext = Tools::getStringXmlAttribute(&contextNode, "lineendcontext");
	context->fallthrough = Tools::getBoolXmlAttribute(&contextNode, "fallthrough", false);
	context->fallthroughContext = Tools::getStringXmlAttribute(&contextNode, "fallthroughcontext");
	context->dynamic = Tools::getBoolXmlAttribute(&contextNode, "dynamic", false);

	for (unsigned int i = 0; i < contextNode.childNodes().length(); i++)
	{
		QDomElement element = contextNode.childNodes().at(i).toElement();
		SyntaxRule* newRule = new SyntaxRule(&element, this);
		if (newRule->isValid())
			context->rules.append(newRule);
		else
		{
			qDebug() << "WARNING: Invalid context rule: " << contextNode.nodeName();
			delete newRule;
		}
	}

	if (!mDefaultContext) mDefaultContext = context;
	mContextMap.insert(context->name, context);

	context->listIndex = mContextList.length();
	mContextList.append(context);
}

void SyntaxDefinition::readHighlightingNode(const QDomElement& highlightingNode)
{
	//	Read all of the keyword lists
	QDomNodeList keywordLists = highlightingNode.elementsByTagName("list");
	for (unsigned int i = 0; i < keywordLists.length(); i++)
	{
		QDomElement keywordList = keywordLists.at(i).toElement();

		QString name = Tools::getStringXmlAttribute(&keywordList, "name");
		QStringList keywords;

		QDomNodeList listEntries = keywordList.childNodes();
		for (unsigned int j = 0; j < listEntries.length(); j++)
			keywords.append(listEntries.at(j).nodeValue());

		mKeywords.insert(name, keywords);
	}

	//	Read all of the contexts
	QDomElement contextsNode = highlightingNode.firstChildElement("contexts");
	for (unsigned int i = 0; i < contextsNode.childNodes().length(); i++)
		readContext(contextsNode.childNodes().at(i).toElement());

	//	Read the itemdatas
	QDomElement itemDatasNode = highlightingNode.firstChildElement("itemdatas");
	for (unsigned int i = 0; i < itemDatasNode.childNodes().length(); i++)
	{
		QDomElement node = itemDatasNode.childNodes().at(i).toElement();
		ItemData* itemData = new ItemData();
		itemData->name = Tools::getStringXmlAttribute(&node, "name");
		itemData->styleName = Tools::getStringXmlAttribute(&node, "styleName");
		itemData->color = Tools::getStringXmlAttribute(&node, "color");
		itemData->selColor = Tools::getStringXmlAttribute(&node, "selColor");
		itemData->italic = Tools::getBoolXmlAttribute(&node, "italic", false);
		itemData->bold = Tools::getBoolXmlAttribute(&node, "bold", false);
		itemData->underline = Tools::getBoolXmlAttribute(&node, "underline", false);
		itemData->strikeout = Tools::getBoolXmlAttribute(&node, "strikeout", false);

		mItemDatas.insert(itemData->name, itemData);
	}
}

void SyntaxDefinition::readGeneralNode(const QDomElement& generalNode)
{
	//	general -> keywords
	QDomElement keywordsNode = generalNode.firstChildElement("keywords").toElement();
	if (!keywordsNode.isNull())
	{
		mCaseSensitiveKeywords = Tools::getBoolXmlAttribute(&keywordsNode, "casesensitive", true);
		mWeakDeliminators = Tools::getStringXmlAttribute(&keywordsNode, "weakdeliminators");
		mAdditionalDeliminators = Tools::getStringXmlAttribute(&keywordsNode, "additionaldeliminator");
		mWordWrapDeliminator = Tools::getStringXmlAttribute(&keywordsNode, "wordwrapdeliminator");
	}

	//	general -> comment
	QDomElement commentsNode = generalNode.firstChildElement("comments").toElement();
	if (!commentsNode.isNull())
	{
		for (unsigned int i = 0; i < commentsNode.childNodes().length(); i++)
		{
			QDomElement commentNode = commentsNode.childNodes().at(i).toElement();
			CommentStyle commentStyle;
			commentStyle.multiline = (Tools::getStringXmlAttribute(&commentNode, "name") == "multiline");
			commentStyle.start = Tools::getStringXmlAttribute(&commentNode, "start");
			commentStyle.end = Tools::getStringXmlAttribute(&commentNode, "end");
			mCommentStyles.append(commentStyle);
		}
	}

	//	general -> folding
	QDomElement foldingNode = generalNode.firstChildElement("folding").toElement();
	if (!foldingNode.isNull())
		mIndentationSensitive = Tools::getBoolXmlAttribute(&foldingNode, "indentationsensitive", false);
}
