#include "syntaxblockdata.h"

SyntaxBlockData::SyntaxBlockData(const QStack<ContextDefLink> stack) :
	QTextBlockUserData()
{
	mStack = stack;
}
