#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include <QHash>
#include <QTextCharFormat>
#include "syntaxdefinition.h"

class QTextDocument;

class SyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	struct Context
	{
		SyntaxDefinition::ContextDef* definition;
	};

	SyntaxHighlighter(QTextDocument* parent, SyntaxDefinition* syntaxDef);

protected:
	void highlightBlock(const QString& text);

private:
	void applyContextLink(const SyntaxDefinition::ContextLink& link, QStack<Context>* contextStack);

	SyntaxDefinition* mSyntaxDefinition;
	QMap<QString, QColor> mDefaultColors;
};

#endif // SYNTAXHIGHLIGHTER_H
