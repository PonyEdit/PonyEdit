#ifndef SYNTAXDEFINITION_H
#define SYNTAXDEFINITION_H

#include <QMap>
#include <QtXml>
#include <QString>
#include <QStringList>

class SyntaxRule;
class SyntaxDefinition
{
public:
	struct CommentStyle
	{
		bool multiline;
		QString start;
		QString end;
	};

	struct ContextDef
	{
		QString attribute;
		QString name;
		QString lineEndContext;
		QString lineBeginContext;
		bool fallthrough;
		QString fallthroughContext;
		bool dynamic;
		QList<SyntaxRule*> rules;

		int listIndex;
	};

	struct ItemData
	{
		QString name;
		QString styleName;
		QString color;
		QString selColor;
		bool italic;
		bool bold;
		bool underline;
		bool strikeout;
	};

	struct KeywordList
	{
		QString name;
		QStringList items;
	};

	SyntaxDefinition(const QString& filename);

	inline bool isValid() const { return mValid; }
	inline ContextDef* getContextByIndex(int index) const { return mContextList.at(index); }
	inline ContextDef* getDefaultContext() const { return mDefaultContext; }
	inline ContextDef* getContext(const QString& name) const { return mContextMap.value(name.toLower()); }
	inline KeywordList* getKeywordList(const QString& name) const { return mKeywordLists.value(name.toLower()); }
	inline ItemData* getItemData(const QString& name) const { return mItemDatas.value(name.toLower()); }

	void addKeywordList(KeywordList* list);
	void addContext(ContextDef* context);
	void addItemData(ItemData* itemData);

	inline void setIndentationSensitive(bool v) { mIndentationSensitive = v; }
	inline void setCaseSensitiveKeywords(bool v) { mCaseSensitiveKeywords = v; }
	inline void setWeakDeliminators(const QString& v) { mWeakDeliminators = v; }
	inline void setAdditionalDeliminators(const QString& v) { mAdditionalDeliminators = v; mDeliminators.append(v); }
	inline void setWordWrapDeliminator(const QString& v) { mWordWrapDeliminator = v; }

	inline bool isDeliminator(const QChar& c) { return mDeliminators.contains(c); }

private:
	bool link();

	bool mValid;
	QString mName;
	QString mSection;
	QStringList mExtensions;

	QMap<QString, KeywordList*> mKeywordLists;
	QMap<QString, ContextDef*> mContextMap;
	ContextDef* mDefaultContext;
	QMap<QString, ItemData*> mItemDatas;

	QList<ContextDef*> mContextList;

	bool mIndentationSensitive;
	bool mCaseSensitiveKeywords;
	QString mWeakDeliminators;
	QString mAdditionalDeliminators;
	QString mWordWrapDeliminator;
	QString mDeliminators;
	QList<CommentStyle> mCommentStyles;
};

extern SyntaxDefinition* gTestSyntaxDef;

#endif // SYNTAXDEFINITION_H
