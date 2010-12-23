#ifndef STRINGTRIE_H
#define STRINGTRIE_H

#include <QVector>

class StringTrie
{
public:
	struct Node
	{
		bool terminator;
		Node* children[256];
	};

    StringTrie();
	void addWord(const QString& word);
	bool containsWord(const QString& word);

	inline const Node* startScan() { return mRoot; }
	inline bool continueScan(const Node** node, unsigned char character) { if ((*node)->children[(unsigned int)character]) { *node = (*node)->children[(unsigned int)character]; return true; } else { return false; } }
	inline bool endScan(const Node* node) { return node->terminator; }

private:
	Node* allocateNode();

	Node* mRoot;

	QVector<Node> mNodeHeap;
	int mNodeHeapCursor;
};

#endif // STRINGTRIE_H
