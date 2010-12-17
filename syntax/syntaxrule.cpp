#include "syntax/syntaxrule.h"
#include "main/tools.h"

QMap<QString, SyntaxRule::Type> SyntaxRule::sTypeMap;
bool SyntaxRule::sTypeMapInitialized = false;

SyntaxRule::SyntaxRule(const QString& name, const QXmlAttributes& attributes)
{
	/*if (!sTypeMapInitialized)
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

	mRegExp = NULL;
	mDefinition = def;
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

		mString = Tools::getStringXmlAttribute(element, "String");
		qDebug() << mType << " string = " << mString;

		mCaseInsensitive = Tools::getBoolXmlAttribute(element, "insensitive", false);
		mDynamic = Tools::getBoolXmlAttribute(element, "dynamic", false);
		mMinimal = Tools::getBoolXmlAttribute(element, "minimal", false);

		mValid = true;
	}
	else
	{
		qDebug() << "Non-existent type: " << type;
	}*/
}

SyntaxRule::~SyntaxRule()
{
	if (mRegExp) delete mRegExp;
}

int SyntaxRule::match(const QString &string, int position)
{
//	int match = 0;

	switch (mType)
	{
	case DetectChar:
		break;

	case Detect2Chars:
		break;

	case AnyChar:
		break;

	case StringDetect:
		break;

	case WordDetect:
		break;

	case RegExpr:
	{
/*		if (!mRegExp) mRegExp = new QRegExp(mString, mCaseInsensitive ? Qt::CaseInsensitive : Qt::CaseSensitive);
		int index = mRegExp->indexIn(string, position, QRegExp::CaretAtZero);
		if (index > -1)
			match = mRegExp->matchedLength();*/
		break;
	}

	case Keyword:
	{
/*		if (mFirstUse) mKeywords = mDefinition->getKeywordList(mString);
		QString substr = string.mid(position);
		foreach (QString keyword, mKeywords)
		{
			if (substr.startsWith(keyword))
			{
				match = keyword.length();
				break;
			}
		}*/
		break;
	}

	case Int:
		break;

	case Float:
		break;

	case HICOct:
		break;

	case HICHex:
		break;

	case HICStringChar:
		break;

	case HICChar:
		break;

	case RangeDetect:
		break;

	case LineContinue:
		break;

	case DetectSpaces:
		break;

	case DetectIdentifier:
		break;
	}

//	mFirstUse = false;
	return 0;
}





























