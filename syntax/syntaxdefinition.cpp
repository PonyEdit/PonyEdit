#include <QtXml>
#include <QFile>
#include "main/tools.h"
#include "syntax/syntaxrule.h"
#include "syntax/syntaxdefinition.h"
#include "syntax/syntaxdefxmlhandler.h"

SyntaxDefinition* gTestSyntaxDef = new SyntaxDefinition("syntaxdefs/perl.xml");

SyntaxDefinition::SyntaxDefinition(const QString& filename)
{
	mValid = false;
	mDefaultContext = NULL;

	QFile file(filename);
	if (file.open(QFile::ReadOnly))
	{
		SyntaxDefXmlHandler handler(this);
		QXmlSimpleReader reader;
		reader.setContentHandler(&handler);
		reader.setErrorHandler(&handler);

		if (reader.parse(&file))
			mValid = true;
	}
}

void SyntaxDefinition::addKeywordList(KeywordList* list)
{
	mKeywordLists.insert(list->name, list);
}

void SyntaxDefinition::addContext(Context* context)
{
	if (mDefaultContext == NULL)
		mDefaultContext = context;
	mContextMap.insert(context->name, context);
}

void SyntaxDefinition::addRule(SyntaxRule* rule)
{
	mRules.append(rule);
}













