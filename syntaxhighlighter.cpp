#include <QtGui>

#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	keywordFormat.setForeground(Qt::blue);
	keywordFormat.setFontWeight(QFont::Bold);
	QStringList keywordPatterns;
	keywordPatterns << "\\bmy\\b" << "\\buse\\b" << "\\binclude\\b"
					<< "\\bif\\b" << "\\for\\b" << "\\bdelete\\b"
					<< "\\belse\\b" << "\\belsif\\b" << "\\bforeach\\b"
					<< "\\bsub\\b" << "\\bour\\b" << "\\bcaller\\b"
					<< "\\bdo\\b" << "\\bdie\\b" << "\\bcontinue\\b"
					<< "\\buntil\\b" << "\\bwhile\\b" << "\\beval\\b"
					<< "\\blast\\b" << "\\bgoto\\b" << "\\bexit\\b"
					<< "\\bnext\\b" << "\\bredo\\b" << "\\breturn\\b"
					<< "\\bdefault\\b" << "\\bwhen\\b" << "\\bgiven\\b"
					<< "\\bimport\\b" << "\\blocal\\b" << "\\bbreak\\b"
					<< "\\bstate\\b" << "\\bpackage\\b" << "\\brequire\\b"
					<< "\\bvar\\b" << "\\bfunction\\b" << "\\bcase\\b"
					<< "\\bthis\\b" << "\\bvoid\\b" << "\\bin\\b"
					<< "\\btypeof\\b";
	foreach (QString pattern, keywordPatterns) {
		rule.pattern = QRegExp(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	singleLineCommentFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegExp("#[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	singleLineCommentFormat.setForeground(Qt::darkGreen);
	rule.pattern = QRegExp("//[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	multiLineCommentFormat.setForeground(Qt::darkGreen);

	quotationFormat.setFontItalic(true);
	quotationFormat.setForeground(Qt::magenta);
	rule.pattern = QRegExp("\".*\"");
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	functionFormat.setFontItalic(true);
	functionFormat.setForeground(Qt::darkBlue);
	rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
	rule.format = functionFormat;
	highlightingRules.append(rule);

	commentStartExpression = QRegExp("(/\\*|^=[A-Za-z0-9]+$)");
	commentEndExpression = QRegExp("(\\*/|^=cut$)");
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
	foreach (HighlightingRule rule, highlightingRules) {
		QRegExp expression(rule.pattern);
		int index = text.indexOf(expression);
		while (index >= 0) {
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = text.indexOf(expression, index + length);
		}
	}
	setCurrentBlockState(0);

	int startIndex = 0;
	if (previousBlockState() != 1)
		startIndex = text.indexOf(commentStartExpression);

	while (startIndex >= 0) {
		int endIndex = text.indexOf(commentEndExpression, startIndex);
		int commentLength;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		} else {
			commentLength = endIndex - startIndex
					+ commentEndExpression.matchedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = text.indexOf(commentStartExpression,
								  startIndex + commentLength);
	}
}
