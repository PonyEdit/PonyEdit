#include "syntaxblockdata.h"

SyntaxBlockData::SyntaxBlockData( const QStack< ContextDefLink > &stack ) {
	mStack = stack;
}
