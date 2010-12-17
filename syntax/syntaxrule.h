#ifndef SYNTAXRULE_H
#define SYNTAXRULE_H

#include <QtXml>
#include <QString>
#include <QMap>
#include "syntaxdefinition.h"

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
		HlCOct,
		HlCHex,
		HlCStringChar,
		HlCChar,
		RangeDetect,
		LineContinue,
		DetectSpaces,
		DetectIdentifier,
		IncludeRules
	};
	static QMap<QString, Type> sTypeMap;
	static bool sTypeMapInitialized;

	SyntaxRule(SyntaxRule* parent, const QString& name, const QXmlAttributes& attributes);
	~SyntaxRule();

	SyntaxRule* getParent() const { return mParent; }
	const QString& getName() const { return mName; }
	inline bool isValid() const { return mValid; }
	int match(const QString& string, int position);

	void addChildRule(SyntaxRule* rule);

private:
	SyntaxRule* mParent;
	QString mName;
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

	//	Duplicate information prepared for faster lookups and the like
	QRegExp* mRegExp;
	QStringList mKeywords;
};

#endif // SYNTAXRULE_H
