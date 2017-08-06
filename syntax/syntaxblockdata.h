#ifndef SYNTAXBLOCKDATA_H
#define SYNTAXBLOCKDATA_H

HIDE_COMPILE_WARNINGS

#include <QTextBlockUserData>
#include <QStack>

UNHIDE_COMPILE_WARNINGS

#include "syntaxdefinition.h"

class SyntaxBlockData : public QTextBlockUserData
{
public:
	explicit SyntaxBlockData(const QStack<ContextDefLink> stack);

	QStack<ContextDefLink> mStack;
};

#endif // SYNTAXBLOCKDATA_H
