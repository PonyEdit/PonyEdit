#include "syntax/syntaxrule.h"
#include "main/tools.h"

QMap<QString, SyntaxRule::Type> SyntaxRule::sTypeMap;
bool SyntaxRule::sTypeMapInitialized = false;

SyntaxRule::SyntaxRule(SyntaxRule* parent, const QString& name, const QXmlAttributes& attributes)
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
		sTypeMap.insert("hlcoct", HlCOct);
		sTypeMap.insert("hlchex", HlCHex);
		sTypeMap.insert("hlcstringchar", HlCStringChar);
		sTypeMap.insert("hlcchar", HlCChar);
		sTypeMap.insert("rangedetect", RangeDetect);
		sTypeMap.insert("linecontinue", LineContinue);
		sTypeMap.insert("detectspaces", DetectSpaces);
		sTypeMap.insert("detectidentifier", DetectIdentifier);
		sTypeMapInitialized = true;
	}

	mName = name;
	mParent = parent;
	mRegExp = NULL;
	mValid = false;

	QString lcName = name.toLower();
	if (sTypeMap.contains(lcName))
	{
		mType  = sTypeMap.value(lcName);

		mAttribute = Tools::getStringXmlAttribute(attributes, "attribute");
		mContext = Tools::getStringXmlAttribute(attributes, "context");
		mBeginRegion = Tools::getStringXmlAttribute(attributes, "beginregion");
		mEndRegion = Tools::getStringXmlAttribute(attributes, "endregion");
		mLookAhead = Tools::getIntXmlAttribute(attributes, "lookahead", 0);
		mFirstNonSpace = Tools::getIntXmlAttribute(attributes, "firstnonspace", 0);
		mColumn = Tools::getIntXmlAttribute(attributes, "column", 0);
		mCharacterA = Tools::getCharXmlAttribute(attributes, "char");
		mCharacterB = Tools::getCharXmlAttribute(attributes, "char1");
		mString = Tools::getStringXmlAttribute(attributes, "string");
		mCaseInsensitive = Tools::getIntXmlAttribute(attributes, "insensitive", 0);
		mDynamic = Tools::getIntXmlAttribute(attributes, "dynamic", 0);
		mMinimal = Tools::getIntXmlAttribute(attributes, "minimal", 0);

		mValid = true;
	}
	else
	{
		qDebug() << "Unrecognized rule type: " << name;
	}
}

SyntaxRule::~SyntaxRule()
{
	if (mRegExp) delete mRegExp;
}

void SyntaxRule::addChildRule(SyntaxRule* rule)
{
	mChildRules.append(rule);
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

	case HlCOct:
		break;

	case HlCHex:
		break;

	case HlCStringChar:
		break;

	case HlCChar:
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





























