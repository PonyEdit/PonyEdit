#include "stringtrie.h"
#define ALLOC_SIZE 1024
#include <QString>

StringTrie::StringTrie()
{
	mNodeHeapCursor = 0;
	mRoot = allocateNode();
}

void StringTrie::addWord(const QString& word)
{
	Node* scan = mRoot;
	for (int i = 0; i < word.length(); i++)
	{
		int c = word.at(i).toAscii();
		if (scan->children[c] == NULL)
			scan->children[c] = allocateNode();
		scan = scan->children[c];
	}
	scan->terminator = true;
}

bool StringTrie::containsWord(const QString& word)
{
	Node* scan = mRoot;
	for (int i = 0; i < word.length(); i++)
	{
		int c = word.at(i).toAscii();
		if (scan->children[c] == NULL)
			return false;
		scan = scan->children[c];
	}
	return scan->terminator;
}

StringTrie::Node* StringTrie::allocateNode()
{
	int oldSize = mNodeHeap.size();
	if (oldSize <= mNodeHeapCursor)
	{
		mNodeHeap.resize(mNodeHeap.size() + ALLOC_SIZE);
		memset(mNodeHeap.data() + oldSize, 0, ALLOC_SIZE * sizeof(Node));
	}
	return ((Node*)mNodeHeap.data()) + mNodeHeapCursor++;
}
