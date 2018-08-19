#include <QDebug>
#include <QString>
#include "stringtrie.h"
#define NODE_HEAP_SIZE 1024

int StringTrie::sNodeHeapCursor = 0;
QList< QVector< StringTrie::Node > * > StringTrie::sNodeHeaps;
QVector< StringTrie::Node > *StringTrie::sCurrentNodeHeap;

StringTrie::StringTrie() {
	sNodeHeapCursor = NODE_HEAP_SIZE;
	mRoot = allocateNode();
}

void StringTrie::addWord( const QString &word ) {
	Node *scan = mRoot;
	for ( auto i : word ) {
		int c = i.toLatin1();
		if ( scan->children[ c ] == nullptr ) {
			scan->children[ c ] = allocateNode();
		}
		scan = scan->children[ c ];
	}
	scan->terminator = true;
}

bool StringTrie::containsWord( const QString &word ) {
	Node *scan = mRoot;
	for ( auto i : word ) {
		int c = i.toLatin1();
		if ( scan->children[ c ] == nullptr ) {
			return false;
		}
		scan = scan->children[ c ];
	}
	return scan->terminator;
}

StringTrie::Node *StringTrie::allocateNode() {
	if ( sNodeHeapCursor >= NODE_HEAP_SIZE ) {
		sNodeHeapCursor = 0;
		auto *newVector = new QVector< Node >( NODE_HEAP_SIZE );
		memset( newVector->data(), 0, newVector->size() * sizeof( Node ) );
		sNodeHeaps.append( newVector );
		sCurrentNodeHeap = newVector;
	}

	return sCurrentNodeHeap->data() + sNodeHeapCursor++;
}

void StringTrie::cleanup() {
	foreach ( QVector< Node > *vector, sNodeHeaps ) {
		delete vector;
	}
}
