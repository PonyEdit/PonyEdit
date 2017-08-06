#ifndef STRINGTRIE_H
#define STRINGTRIE_H

#include <QVector>

class StringTrie {
public:
	struct Node {
		bool  terminator;
		Node *children[ 256 ];
	};

	StringTrie();
	void addWord( const QString &word );
	bool containsWord( const QString &word );

	inline const Node *startScan() {
		return mRoot;
	}
	inline bool continueScan( const Node **node, unsigned char character ) {
		if ( ( *node )->children[ (unsigned int) character ] ) {
			*node = ( *node )->children[ (unsigned int) character ];
			return true;
		} else {
			return false;
		}
	}
	inline bool endScan( const Node *node ) {
		return node->terminator;
	}

	static void cleanup();

private:
	Node *allocateNode();
	Node *mRoot;

	static QList< QVector< Node > * > sNodeHeaps;
	static QVector< Node > *          sCurrentNodeHeap;
	static int                        sNodeHeapCursor;
};

#endif // STRINGTRIE_H
