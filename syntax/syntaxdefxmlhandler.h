#ifndef SYNTAXDEFXMLHANDLER_H
#define SYNTAXDEFXMLHANDLER_H

#include <QXmlContentHandler>
#include <QXmlErrorHandler>
#include "syntaxdefinition.h"

class SyntaxDefXmlHandler : public QXmlDefaultHandler
{
public:
	SyntaxDefXmlHandler(SyntaxDefinition* definition);

	bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
	bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
	bool characters(const QString &ch);

private:
	enum ElementFlags
	{
		None         = 0x0000,
		Language     = 0x0001,
		Highlighting = 0x0002,
		List         = 0x0004,
		Item         = 0x0008,
		Contexts     = 0x0010,
		Context      = 0x0020,
		Rule         = 0x0040,
		ItemDatas    = 0x0080,
		General      = 0x0100,
		Comments     = 0x0200
	};

	SyntaxDefinition* mDefinition;
	int mCurrentBlocks;

	SyntaxDefinition::KeywordList* mKeywordList;
	SyntaxDefinition::Context* mContext;
	SyntaxRule* mRule;
};

#endif // SYNTAXDEFXMLHANDLER_H
