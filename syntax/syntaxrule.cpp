#include "syntax/syntaxrule.h"
#include "main/tools.h"
#include "syntaxdefmanager.h"

QMap<QString, SyntaxRule::Type>* SyntaxRule::sTypeMap;
bool SyntaxRule::sTypeMapInitialized = false;

SyntaxRule::SyntaxRule(SyntaxRule* parent, const QString& name, const QXmlAttributes& attributes)
{
	if (!sTypeMapInitialized)
	{
		sTypeMap = new QMap<QString, Type>();
		sTypeMap->insert("detectchar", DetectChar);
		sTypeMap->insert("detect2chars", Detect2Chars);
		sTypeMap->insert("anychar", AnyChar);
		sTypeMap->insert("stringdetect", StringDetect);
		sTypeMap->insert("worddetect", WordDetect);
		sTypeMap->insert("regexpr", RegExpr);
		sTypeMap->insert("keyword", Keyword);
		sTypeMap->insert("int", Int);
		sTypeMap->insert("float", Float);
		sTypeMap->insert("hlcoct", HlCOct);
		sTypeMap->insert("hlchex", HlCHex);
		sTypeMap->insert("hlcstringchar", HlCStringChar);
		sTypeMap->insert("hlcchar", HlCChar);
		sTypeMap->insert("rangedetect", RangeDetect);
		sTypeMap->insert("linecontinue", LineContinue);
		sTypeMap->insert("detectspaces", DetectSpaces);
		sTypeMap->insert("detectidentifier", DetectIdentifier);
		sTypeMap->insert("includerules", IncludeRules);
		sTypeMapInitialized = true;
	}

	mName = name;
	mParent = parent;
	mValid = false;
	mLinked = false;

	QString lcName = name.toLower();
	if (sTypeMap->contains(lcName))
	{
		mType  = sTypeMap->value(lcName);

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
		mColumn = Tools::getIntXmlAttribute(attributes, "column", -1);
		mCharacterA = Tools::getCharXmlAttribute(attributes, "char");
		mCharacterB = Tools::getCharXmlAttribute(attributes, "char1");
		mString = Tools::getStringXmlAttribute(attributes, "string");

		int tmpSensitivity = Tools::getIntXmlAttribute(attributes, "insensitive", -1);
		mCaseSensitivity = tmpSensitivity < 0 ? -1 : (tmpSensitivity ? Qt::CaseInsensitive : Qt::CaseSensitive);

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

SyntaxRule::SyntaxRule(SyntaxRule* parent, const QSharedPointer<SyntaxRule>& other, bool duplicateChildren, bool maintainLinks)
{
	mParent = parent;

	mName = other->mName;
	mType = other->mType;
	mValid = other->mValid;

	mAttribute = other->mAttribute;
	mContext = other->mContext;
	mBeginRegion = other->mBeginRegion;
	mEndRegion = other->mEndRegion;
	mLookAhead = other->mLookAhead;
	mFirstNonSpace = other->mFirstNonSpace;
	mColumn = other->mColumn;

	mCharacterA = other->mCharacterA;
	mCharacterB = other->mCharacterB;
	mString = other->mString;
	mCaseSensitivity = other->mCaseSensitivity;
	mDynamic = other->mDynamic;
	mMinimal = other->mMinimal;
	mIncludeAttrib = other->mIncludeAttrib;

	if (maintainLinks)
	{
		mAttributeLink = other->mAttributeLink;
		mRegExp = other->mRegExp;
		mKeywordLink = other->mKeywordLink;
		mContextLink = other->mContextLink;
		mDynamicCharIndex = other->mDynamicCharIndex;
		mDynamicStringSlots = other->mDynamicStringSlots;
	}

	if (duplicateChildren)
	{
		foreach (const QSharedPointer<SyntaxRule>& otherChild, other->mChildRules)
			mChildRules.append(QSharedPointer<SyntaxRule>(new SyntaxRule(this, otherChild, duplicateChildren, maintainLinks)));
	}
	else
		mChildRules = other->mChildRules;
}

SyntaxRule::~SyntaxRule()
{
}

void SyntaxRule::addChildRule(QSharedPointer<SyntaxRule> rule)
{
	mChildRules.append(rule);
}

void SyntaxRule::applyDynamicCaptures(const QStringList& captures)
{
	if (mType == DetectChar || mType == Detect2Chars)
	{
		if (captures.length() > mDynamicCharIndex)
			mCharacterA = captures[mDynamicCharIndex][0];
	}
	else
	{
		foreach (DynamicSlot slot, mDynamicStringSlots)
		{
			if (captures.length() > slot.id)
			{
				QString insert = captures[slot.id];
				insert.replace(QRegExp("(\\W)"), "\\\\1");
				mString.insert(slot.pos, insert);
			}
		}

		if (mType == RegExpr)
			prepareRegExp();
	}
}

bool SyntaxRule::link(SyntaxDefinition* def)
{
	if (mLinked) return true;

	mDefinition = def;

	if (mAttribute.isEmpty())
		mAttributeLink = NULL;
	else
	{
		mAttributeLink = def->getItemData(mAttribute);
		if (!mAttributeLink)
			qDebug() << "Warning: Failed to link attribute: " << mAttribute;
	}

	if (!def->linkContext(mContext, &mContextLink))
		return false;

	//	Link all children too
	foreach (QSharedPointer<SyntaxRule> rule, mChildRules)
		if (!rule->link(def))
			return false;

	//	Rule specific link-ups
	switch (mType)
	{
	case RegExpr:
		if (!mDynamic) prepareRegExp();
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

	if (getCaseSensitivity() == Qt::CaseSensitive)
	{
		mString = mString.toLower();
		mCharacterA = mCharacterA.toLower();
		mCharacterB = mCharacterB.toLower();
	}

	//	If this is dynamic, pre-calculate the dynamic replacement goop
	if (mDynamic)
	{
		mDynamicCharIndex = 0;
		if (mType == DetectChar || mType == Detect2Chars)
			mDynamicCharIndex = (mCharacterA >= '0' && mCharacterA <= '9' ? mCharacterA.toAscii() - '0' : 0);
		else
		{
			int index = 0;
			while ((index = mString.indexOf('%', index)) > -1)
			{
				if (index == mString.length() - 1) break;
				char c = mString[index + 1].toLatin1();

				if (c == '%')
					mString.remove(index, 1);
				else
				{
					if (c >= '0' && c <= '9')
						this->mDynamicStringSlots.push_front(DynamicSlot(index, c - '0'));
					mString.remove(index, 2);
				}
				index++;
			}
		}
	}

	mLinked = true;
	return true;
}

void SyntaxRule::prepareRegExp()
{
	mRegExp = QRegExp(mString, getCaseSensitivity());
	if (mMinimal)
		mRegExp.setMinimal(true);
}

int SyntaxRule::match(const QString &string, int position)
{
	int match = 0;

	if (mFirstNonSpace)
		for(int i = 0; i < position; i++)
			if (!string[i].isSpace())
				return 0;

	if (mColumn > -1 && position != mColumn)
		return 0;

	switch (mType)
	{
	case DetectChar:
		if (getCaseSensitivity() == Qt::CaseInsensitive)
			match = (string.at(position).toLower() == mCharacterA ? 1 : 0);
		else
			match = (string.at(position) == mCharacterA ? 1 : 0);
		break;

	case Detect2Chars:
		if (position < string.length() - 1)
		{
			if (getCaseSensitivity() == Qt::CaseInsensitive)
				match = (string.at(position).toLower() == mCharacterA && string.at(position + 1).toLower() == mCharacterB ? 2 : 0);
			else
				match = (string.at(position) == mCharacterA && string.at(position + 1) == mCharacterB ? 2 : 0);
		}
		break;

	case AnyChar:
		if (mString.contains(string.at(position)))
			match = 1;
		break;

	case StringDetect:
		if (Tools::compareSubstring(string, mString, position, getCaseSensitivity()))
			match = mString.length();
		break;

	case WordDetect:
		if (position == 0 || mDefinition->isDeliminator(string.at(position - 1)))
			if (position == string.length() - mString.length() || mDefinition->isDeliminator(string.at(position + mString.length())))
				if (Tools::compareSubstring(string, mString, position, getCaseSensitivity()))
					match = mString.length();
		break;

	case RegExpr:
	{
		int index = mRegExp.indexIn(string, position, QRegExp::CaretAtZero);
		if (index == position) match = mRegExp.matchedLength();
		break;
	}

	case IncludeRules:
		qDebug() << "Warning: IncludeRules left in the SyntaxDefinition after linking! :(";
		break;

	case Keyword:
		if (position == 0 || mDefinition->isDeliminator(string.at(position - 1)))
		{
			const QChar* s = string.constData() + position;
			const StringTrie::Node* scan = mKeywordLink->items.startScan();
			int length = 0;


			while (!s->isNull() && !mDefinition->isDeliminator(*s))
			{
				if (mKeywordLink->items.continueScan(&scan, static_cast<unsigned char>(s->toLatin1())))
					length++;
				else
				{
					length = 0;
					break;
				}

				s++;
			}
			if (length && mKeywordLink->items.endScan(scan))
				match = length;
		}
		break;

	case DetectSpaces:
		while (position + match < string.length() && string.at(position + match).isSpace())
			match++;
		break;

	case HlCOct:
		//	Octals start with 0, but have no x like hex. eg; 01562 is octal.
		if (string.at(position) == '0')
		{
			int lookahead = 1;
			const QChar* s = string.constData() + position + 1;
			while (position + lookahead < string.length() && (*s >= '0') && (*s <= '7'))
				s++,lookahead++;

			if (lookahead > 1)
				match = lookahead;
		}
		break;

	case Int:
	{
		//	Ints are any numbers 0-9
		const QChar* s = string.constData() + position;

		int extra = 0;
		if (*s == '-')
			extra++,s++;
		while (position + match < string.length() && (s->isDigit()))
			s++,match++;
		if (match) match += extra;
		break;
	}

	case DetectIdentifier:
	{
		//	[a-zA-Z_][a-zA-Z0-9_]*
		const QChar* s = string.constData() + position;
		if ((*s >= 'a' && *s <= 'z') || (*s >= 'A' &&  *s <= 'Z') || *s == '_')
		{
			s++;
			match = 1;
			while (position + match < string.length() && ((*s >= 'a' && *s <= 'z') || (*s >= 'A' &&  *s <= 'Z') || *s == '_' || (*s >= '0' && *s <= '9')))
				s++,match++;
		}
		break;
	}

	case Float:
	{
		//	[-][0-9]+.[0-9]#+e[0-9]+
		const QChar* s = string.constData() + position;
		int extra = 0;
		bool seenDecimal = false;
		if (*s == '-')
			extra++,s++;
		if (!s->isNull() && s->isDigit())
		{
			match++,s++;
			while (!s->isNull() && (s->isDigit() || *s == '.' || *s == 'e' || *s == 'E'))
			{
				if (*s == '.') seenDecimal = true;
				match++,s++;
			}
			match += extra;
		}
		if (!seenDecimal) match = false;
		break;
	}

	case HlCHex:
	{
		//	[-]0x[0-9]+
		const QChar* s = string.constData() + position;
		int extra = 0;
		if (*s == '-')
			extra++,s++;
		if (*(s++) == '0')
		{
			if (*(s++) == 'x')
			{
				while (!s->isNull() && (s->isDigit() || (*s >= 'A' && *s <= 'F') || (*s >= 'a' && *s <= 'f')))
					match++,s++;
				if (match > 0)
					match += 2 + extra;
			}
		}
		break;
	}

	case HlCStringChar:
		match = detectStringChar(string, position);
		break;

	case HlCChar:
	{
		const QChar* s = string.constData() + position;
		if (*s == '\'')
		{
			s++;
			if (s->isNull()) return 0;

			int innerLen = 1;
			if (*s == '\\')
				innerLen = detectStringChar(string, position + 1);

			s += innerLen;
			if (!s->isNull())
				match = innerLen + 2;
		}
		break;
	}

	case RangeDetect:
		if (string[position] == mCharacterA)
		{
			int end = string.indexOf(mCharacterB, position + 1);
			if (end > position)
				match = end - position;
		}
		break;

	case LineContinue:
		if (position == string.length() - 1)
			if (string.at(position) == '\\')
				match = 1;
		break;
	}

	//	If this rule matches, check for child matches too
	if (match && position + match < string.length())
	{
		foreach (QSharedPointer<SyntaxRule> rule, mChildRules)
		{
			int childMatch = rule->match(string, position + match);
			if (childMatch > 0)
			{
				match += childMatch;
				break;
			}
		}
	}

	return match;
}

int SyntaxRule::detectStringChar(const QString& string, int position)
{
	const QChar* s = string.constData() + position;
	if (*s == '\\')
	{
		s++;
		if (s->isNull()) return 0;

		switch (s->toLatin1())
		{
		case 'a': case 'b': case 'f': case 'n': case 'r':
		case 't': case 'v': case '\'': case '"': case '\\':
		case '?':
			return 2;
		}

		int nLength = 0;
		if (*s == 'x')
		{
			while (nLength <= 4 && !s->isNull() && (s->isDigit() || (*s >= '0' && *s <= '9') || (*s >= 'a' && *s <= 'f') || (*s >= 'A' && *s <= 'F')))
				s++;nLength++;
			nLength = (nLength & 0x10) + 2;
		}
		else
		{
			while (nLength <= 3 && !s->isNull() && *s >= '0' && *s <= '9')
				s++,nLength++;
			if (nLength != 3)
				nLength = 0;
			else
				nLength++;
		}

		return nLength;
	}
	return 0;
}




























