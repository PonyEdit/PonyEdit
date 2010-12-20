#ifndef SYNTAXBLOCKDATA_H
#define SYNTAXBLOCKDATA_H

#include <QTextBlockUserData>
#include <QStack>
#include "syntaxdefinition.h"

class SyntaxBlockData : public QTextBlockUserData
{
public:
	explicit SyntaxBlockData(const QStack<ContextDefLink> stack);

	QStack<ContextDefLink> mStack;
};

#endif // SYNTAXBLOCKDATA_H
