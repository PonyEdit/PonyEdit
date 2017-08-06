#ifndef SYNTAXBLOCKDATA_H
#define SYNTAXBLOCKDATA_H

#include "syntaxdefinition.h"
#include <QStack>
#include <QTextBlockUserData>

class SyntaxBlockData : public QTextBlockUserData {
public:
	explicit SyntaxBlockData( const QStack< ContextDefLink > stack );

	QStack< ContextDefLink > mStack;
};

#endif // SYNTAXBLOCKDATA_H
