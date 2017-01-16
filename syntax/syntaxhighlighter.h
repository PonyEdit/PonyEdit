#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

HIDE_COMPILE_WARNINGS

#include <QSyntaxHighlighter>
#include <QHash>
#include <QTextCharFormat>

UNHIDE_COMPILE_WARNINGS

#include "syntax/syntaxdefinition.h"

#define MAX_HIGHLIGHT_LENGTH 2000

class QTextDocument;

class SyntaxHighlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	SyntaxHighlighter(QTextDocument* parent, SyntaxDefinition* syntaxDef);

	SyntaxHighlighter(SyntaxHighlighter const&) = delete;
	SyntaxHighlighter& operator=(SyntaxHighlighter const&) = delete;

	inline SyntaxDefinition* getSyntaxDefinition() const { return mSyntaxDefinition; }
	void setSyntaxDefinition(SyntaxDefinition* definition);

protected:
	void highlightBlock(const QString& text);

private:
	ContextDefLink duplicateDynamicContext(const ContextDefLink& source, const QStringList& captures) const;
	void replaceDynamicRules(SyntaxRule* parent, QList<QSharedPointer<SyntaxRule> >* ruleList, const QStringList& captures) const;
	void applyContextLink(const SyntaxDefinition::ContextLink* contextLink, QStack<ContextDefLink>* contextStack, QStringList* dynamicCaptures);

	SyntaxDefinition* mSyntaxDefinition;
	QMap<QString, QColor> mDefaultColors;
};

#endif // SYNTAXHIGHLIGHTER_H
