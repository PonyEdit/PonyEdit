#ifndef SYNTAXDEFXMLHANDLER_H
#define SYNTAXDEFXMLHANDLER_H

#include <QXmlContentHandler>
#include <QXmlErrorHandler>
#include "syntaxdefinition.h"
#include "syntaxdefmanager.h"

class SyntaxDefXmlHandler : public QXmlDefaultHandler {
	public:
// Constructor with definition = load full file, into definition
		SyntaxDefXmlHandler( SyntaxDefinition* definition );

// Constructor with manager record = load basic info only, into record
		SyntaxDefXmlHandler( SyntaxDefManager::Record* record );

		bool startElement( const QString &namespaceURI,
		                   const QString &localName,
		                   const QString &qName,
		                   const QXmlAttributes &atts );
		bool endElement( const QString &namespaceURI, const QString &localName, const QString &qName );
		bool characters( const QString &ch );

		QString errorString() const;

	private:
		void packManagerRecord( const QXmlAttributes& atts );

		enum ElementFlags {
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
		SyntaxDefManager::Record* mRecord;

		int mCurrentBlocks;

		SyntaxDefinition::KeywordList* mKeywordList;
		SyntaxDefinition::ContextDef* mContext;
		SyntaxRule* mRule;
};

#endif  // SYNTAXDEFXMLHANDLER_H
