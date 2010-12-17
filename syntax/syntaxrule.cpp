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
		sTypeMap.insert("includerules", IncludeRules);
		sTypeMapInitialized = true;
	}

	mName = name;
	mParent = parent;
	mValid = false;

	QString lcName = name.toLower();
	if (sTypeMap.contains(lcName))
	{
		mType  = sTypeMap.value(lcName);

		if (mType == IncludeRules && mParent != NULL)
		{
			qDebug() << "Warning: Include inside parent rule; disregarding!";
			return;
		}

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
		mIncludeAttrib = Tools::getIntXmlAttribute(attributes, "includeAttrib", 0);

		mValid = true;
	}
	else
	{
		qDebug() << "Unrecognized rule type: " << name;
	}
}

SyntaxRule::~SyntaxRule()
{
}

void SyntaxRule::addChildRule(SyntaxRule* rule)
{
	mChildRules.append(rule);
}

bool SyntaxRule::link(SyntaxDefinition* def)
{
	if (mAttribute.isEmpty())
		mAttributeLink = NULL;
	else
	{
		mAttributeLink = def->getItemData(mAttribute);
		if (!mAttributeLink)
		{
			qDebug() << "Failed to link attribute: " << mAttribute;
			return false;
		}
	}

	//	Context can be "#stay" (no change), "#pop#pop..." (pop x times), or a context name
	mContextLink = NULL;
	mContextPopCount = 0;
	if (mContext.startsWith('#') || mContext.isEmpty())
	{
		if (mContext.startsWith("#pop", Qt::CaseInsensitive) == 0)
			mContextPopCount = mContext.count('#');
	}
	else
	{
		mContextLink = def->getContext(mContext);
		if (!mContextLink)
		{
			qDebug() << "Failed to link context: " << mContext;
			return false;
		}
	}

	//	Link all children too
	foreach (SyntaxRule* rule, mChildRules)
		if (!rule->link(def))
			return false;

	//	Rule specific link-ups
	switch (mType)
	{
	case RegExpr:
		mRegExp = QRegExp(mString, mCaseInsensitive ? Qt::CaseInsensitive : Qt::CaseSensitive);
		break;

	case Keyword:
		mKeywordLink = def->getKeywordList(mString);
		if (!mKeywordLink)
		{
			qDebug() << "Failed to link keyword list: " << mString;
			return false;
		}
		break;

	default:break;
	}

	return true;
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

	case IncludeRules:
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





























