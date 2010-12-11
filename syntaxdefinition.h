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

	SyntaxDefinition(const QString& filename);

	inline bool isValid() const { return mValid; }

private:
	void readContext(const QDomElement& contextNode);
	void readHighlightingNode(const QDomElement& highlightingNode);
	void readGeneralNode(const QDomElement& generalNode);
	bool readXml(QDomDocument* document);

	bool mValid;
	QString mName;
	QString mSection;
	QStringList mExtensions;

	QMap<QString, QStringList> mKeywords;
	QMap<QString, Context*> mContexts;
	QMap<QString, ItemData*> mItemDatas;

	bool mIndentationSensitive;
	bool mCaseSensitiveKeywords;
	QString mWeakDeliminators;
	QString mAdditionalDeliminators;
	QString mWordWrapDeliminator;
	QList<CommentStyle> mCommentStyles;
};

#endif // SYNTAXDEFINITION_H
