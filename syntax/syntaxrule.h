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
	inline Type getType() const { return mType; }
	inline const QString& getStringAttribute() const { return mString; }
	inline const bool getIncludeAttrib() const { return mIncludeAttrib; }
	inline const QString& getContext() const { return mContext; }
	inline SyntaxDefinition::ItemData* getAttributeLink() const { return mAttributeLink; }
	inline bool isLookAhead() const { return mLookAhead; }
	inline bool getPopCount() const { return mContextPopCount; }
	inline const SyntaxDefinition::ContextLink& getContextLink() const { return mContextLink; }

	int match(const QString& string, int position);
	void addChildRule(SyntaxRule* rule);
	bool link(SyntaxDefinition* def);

private:
	SyntaxDefinition* mDefinition;
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
	bool mIncludeAttrib;

	QList<SyntaxRule*> mChildRules;

	//	Duplicate information prepared for faster lookups and the like
	SyntaxDefinition::ItemData* mAttributeLink;
	int mContextPopCount;
	QRegExp mRegExp;
	SyntaxDefinition::KeywordList* mKeywordLink;
	SyntaxDefinition::ContextLink mContextLink;
};

#endif // SYNTAXRULE_H
