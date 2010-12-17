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

	struct Context
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
	inline Context* getContextByIndex(int index) const { return mContextList.at(index); }
	inline Context* getDefaultContext() const { return mDefaultContext; }
	inline Context* getContext(const QString& name) const { return mContextMap.value(name); }
	inline KeywordList* getKeywordList(const QString& name) const { return mKeywordLists.value(name); }
	inline ItemData* getItemData(const QString& name) const { return mItemDatas.value(name); }

	void addKeywordList(KeywordList* list);
	void addContext(Context* context);
	void addRule(SyntaxRule* rule);
	void addItemData(ItemData* itemData);

	void setIndentationSensitive(bool v) { mIndentationSensitive = v; }
	void setCaseSensitiveKeywords(bool v) { mCaseSensitiveKeywords = v; }
	void setWeakDeliminators(const QString& v) { mWeakDeliminators = v; }
	void setAdditionalDeliminators(const QString& v) { mAdditionalDeliminators = v; }
	void setWordWrapDeliminator(const QString& v) { mWordWrapDeliminator = v; }

private:
	bool link();

	bool mValid;
	QString mName;
	QString mSection;
	QStringList mExtensions;

	QMap<QString, KeywordList*> mKeywordLists;
	QMap<QString, Context*> mContextMap;
	Context* mDefaultContext;
	QList<SyntaxRule*> mRules;
	QMap<QString, ItemData*> mItemDatas;

	QList<Context*> mContextList;

	bool mIndentationSensitive;
	bool mCaseSensitiveKeywords;
	QString mWeakDeliminators;
	QString mAdditionalDeliminators;
	QString mWordWrapDeliminator;
	QList<CommentStyle> mCommentStyles;
};

extern SyntaxDefinition* gTestSyntaxDef;

#endif // SYNTAXDEFINITION_H
