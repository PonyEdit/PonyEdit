#ifndef SYNTAXRULE_H
#define SYNTAXRULE_H

#include <QtXml>
#include <QString>
#include <QMap>

class SyntaxRule
{
public:
	enum Type
	{
		DetectChar,
		Detect2Chars,
		AnyChar,
		StringDetect,
		WordDetect,
		RegExpr,
		Keyword,
		Int,
		Float,
		HICOct,
		HICHex,
		HICStringChar,
		HICChar,
		RangeDetect,
		LineContinue,
		DetectSpaces,
		DetectIdentifier
	};
	static QMap<QString, Type> sTypeMap;
	static bool sTypeMapInitialized;

	SyntaxRule(QDomElement* element);

private:
	Type mType;
	bool mValid;

	QString mAttribute;
	QString mContext;
	QString mBeginRegion;  // for code folding
	QString mEndRegion;    // for code folding
	bool mLookAhead;
	bool mFirstNonSpace;
	int mColumn;

	QChar mCharacterA;
	QChar mCharacterB;
	QString mString;
	bool mCaseInsensitive;
	bool mDynamic;
	bool mMinimal;

	QList<SyntaxRule*> mChildRules;
};

#endif // SYNTAXRULE_H
