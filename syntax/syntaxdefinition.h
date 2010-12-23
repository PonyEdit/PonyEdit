#ifndef SYNTAXDEFINITION_H
#define SYNTAXDEFINITION_H

#include <QMap>
#include <QtXml>
#include <QString>
#include <QStringList>
#include <QSharedPointer>

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

	struct ContextDef;
	struct ContextLink
	{
		ContextLink() : popCount(0), contextDef(NULL) {}
		int popCount;
		QSharedPointer<ContextDef> contextDef;
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

	struct ContextDef
	{
		ContextDef();
		~ContextDef();

		QString attribute;
		QString name;
		QString lineEndContext;
		QString lineBeginContext;
		bool fallthrough;
		QString fallthroughContext;
		bool dynamic;
		QList<QSharedPointer<SyntaxRule> > rules;

		ContextLink fallthroughContextLink;
		ContextLink lineBeginContextLink;
		ContextLink lineEndContextLink;

		int listIndex;
		ItemData* attributeLink;
	};

	struct KeywordList
	{
		QString name;
		QStringList items;
	};

	SyntaxDefinition(const QString& filename);

	inline bool isValid() const { return mValid; }
	inline const QSharedPointer<ContextDef>& getContextByIndex(int index) const { return mContextList.at(index); }
	inline const QSharedPointer<ContextDef>& getDefaultContext() const { return mDefaultContext; }
	inline QSharedPointer<ContextDef> getContext(const QString& name) const { return mContextMap.value(name.toLower()); }
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

	bool linkContext(const QString& context, ContextLink* link);

	Qt::CaseSensitivity getKeywordCaseSensitivity() { return mCaseSensitiveKeywords ? Qt::CaseSensitive : Qt::CaseInsensitive; }

private:
	bool link();

	bool mValid;

	QMap<QString, KeywordList*> mKeywordLists;
	QMap<QString, QSharedPointer<ContextDef> > mContextMap;
	QSharedPointer<ContextDef> mDefaultContext;
	QMap<QString, ItemData*> mItemDatas;

	QList<QSharedPointer<ContextDef> > mContextList;

	bool mIndentationSensitive;
	bool mCaseSensitiveKeywords;
	QString mWeakDeliminators;
	QString mAdditionalDeliminators;
	QString mWordWrapDeliminator;
	QString mDeliminators;
	QList<CommentStyle> mCommentStyles;
};

typedef QSharedPointer<SyntaxDefinition::ContextDef> ContextDefLink;

#endif // SYNTAXDEFINITION_H
