#include "syntaxrule.h"
#include "tools.h"

QMap<QString, SyntaxRule::Type> SyntaxRule::sTypeMap;
bool SyntaxRule::sTypeMapInitialized = false;

SyntaxRule::SyntaxRule(QDomElement* element)
{
	if (!sTypeMapInitialized)
	{
		sTypeMap.insert("detectchar", DetectChar);
		sTypeMap.insert("detect2chars", Detect2Chars);
		sTypeMap.insert("anychar", AnyChar);
		sTypeMap.insert("stringdetect", StringDetect);
		sTypeMap.insert("worddetect", WordDetect);
		sTypeMap.insert("regexpr", RegExpr);
		sTypeMap.insert("keyword", Keyword);
		sTypeMap.insert("int", Int);
		sTypeMap.insert("float", Float);
		sTypeMap.insert("hicoct", HICOct);
		sTypeMap.insert("hichex", HICHex);
		sTypeMap.insert("hicstringchar", HICStringChar);
		sTypeMap.insert("hicchar", HICChar);
		sTypeMap.insert("rangedetect", RangeDetect);
		sTypeMap.insert("linecontinue", LineContinue);
		sTypeMap.insert("detectspaces", DetectSpaces);
		sTypeMap.insert("detectidentifier", DetectIdentifier);
	}

	mValid = false;
	QString type = element->nodeName().toLower();
	if (sTypeMap.contains(type))
	{
		mType = sTypeMap.value(type);
		mAttribute = Tools::getStringXmlAttribute(element, "attribute");
		mContext = Tools::getStringXmlAttribute(element, "context");
		mBeginRegion = Tools::getStringXmlAttribute(element, "beginregion");
		mEndRegion = Tools::getStringXmlAttribute(element, "");
		mLookAhead = Tools::getBoolXmlAttribute(element, "", false);
		mFirstNonSpace = Tools::getBoolXmlAttribute(element, "", false);
		mColumn = Tools::getIntXmlAttribute(element, "", -1);
		mCharacterA = Tools::getCharXmlAttribute(element, "char");
		mCharacterB = Tools::getCharXmlAttribute(element, "char1");
		mString = Tools::getStringXmlAttribute(element, "string");
		mCaseInsensitive = Tools::getBoolXmlAttribute(element, "insensitive", false);
		mDynamic = Tools::getBoolXmlAttribute(element, "dynamic", false);
		mMinimal = Tools::getBoolXmlAttribute(element, "minimal", false);

		mValid = true;
	}
}
