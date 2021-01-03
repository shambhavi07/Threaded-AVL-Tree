/*avlt.h*/

//
// Threaded AVL tree
//

#pragma once

#include <iostream>
#include <vector>

using namespace std;

template<typename KeyT, typename ValueT>
class avlt
{

private:

struct NODE
{
	KeyT   Key;
	ValueT Value;
	NODE*  Left;
	NODE*  Right;
	bool   isThreaded; // true => Right is a thread, false => non-threaded
	int    Height;     // height of tree rooted at this node
};

NODE* Root;  // pointer to root node of tree (nullptr if empty)
int   Size;  // # of nodes in the tree (0 if empty)
NODE* Next;

int _max(int a, int b)
{
	return (a > b)?a:b;
}

NODE* _getActualLeft(NODE* cur) const
{
	return cur->Left;
}

NODE* _getActualRight(NODE* cur) const
{
	if(cur->isThreaded)
		return nullptr;
	return cur->Right;
}

NODE* _search(NODE* cur, KeyT key) const
{
	if(cur == nullptr)
		return nullptr;

	if(cur->Key == key)
		return cur;
	else if(cur->Key < key)
		return _search(_getActualRight(cur), key);
	else
		return _search(_getActualLeft(cur), key);
}

void _free(NODE* cur)
{
	if(cur == nullptr)
		return;
	_free(_getActualRight(cur));
	_free(_getActualLeft(cur));

	delete cur;
}

NODE* _copy(NODE* cur, NODE* &threadNode)
{
	if(cur == nullptr)
	{
		threadNode = nullptr;
		return nullptr;
	}

	NODE *threadFromLeftNode;
	NODE *leftChild = _copy(_getActualLeft(cur), threadFromLeftNode);
	NODE *rightChild = _copy(_getActualRight(cur), threadNode);

	NODE *node = new NODE();
	node->Key = cur->Key;
	node->Value = cur->Value;
	node->Height = cur->Height;
	node->isThreaded = cur->isThreaded;
	node->Left = leftChild;
	node->Right = rightChild;

	if(threadFromLeftNode != nullptr)
		threadFromLeftNode->Right = node;
	if(node->isThreaded)
		threadNode = node;

	return node;
}

int _getHeight(NODE* cur)
{
	if(cur == nullptr)
		return -1;
	return cur->Height;
}

NODE* _rightRotate(NODE* node)
{
	NODE *L = _getActualLeft(node);
	NODE *B = _getActualRight(L);

	L->isThreaded = false;

	L->Right = node;
	node->Left = B;

	node->Height = 1 + _max(_getHeight(_getActualLeft(node)), _getHeight(_getActualRight(node)));
	L->Height = 1 + _max(_getHeight(_getActualLeft(L)), _getHeight(_getActualRight(L)));

	return L;
}

NODE* _leftRotate(NODE* node)
{
	NODE *R = _getActualRight(node);
	NODE *B = _getActualLeft(R);

	R->Left = node;
	node->Right = B;
	if(B == nullptr)
	{
		node->Right = R;
		node->isThreaded = true;
	}

	node->Height = 1 + _max(_getHeight(_getActualLeft(node)), _getHeight(_getActualRight(node)));
	R->Height = 1 + _max(_getHeight(_getActualLeft(R)), _getHeight(_getActualRight(R)));

	return R;
}

NODE* _insert(NODE* node, KeyT key, ValueT value)
{
	if(node == nullptr)
	{
		NODE* n = new NODE();
		n->Key = key;
		n->Value = value;
		n->Left = nullptr;
		n->Right = nullptr;
		n->isThreaded = true;
		n->Height = 0;
		++Size;
		return n;
	}

	if(key < node->Key)
	{
		NODE *insertNode = _insert(_getActualLeft(node), key, value);
		if(insertNode->isThreaded)
			insertNode->Right = node;
		node->Left = insertNode;
	}
	else if(key > node->Key)
	{
		NODE *insertNode = _insert(_getActualRight(node), key, value);
		if(node->isThreaded)
		{
			insertNode->Right = node->Right;
			node->isThreaded = false;
			insertNode->isThreaded = true;
		}
		node->Right = insertNode;
	}
	else
		return node;

	node->Height = 1 + _max(_getHeight(_getActualLeft(node)), _getHeight(_getActualRight(node)));

	int heightDiff = _getHeight(_getActualLeft(node)) - _getHeight(_getActualRight(node));

	// CASE 1
	if(heightDiff > 1 && key < node->Left->Key)
		return _rightRotate(node);
	// CASE 2
	else if(heightDiff > 1 && key > node->Left->Key)
	{
		node->Left = _leftRotate(node->Left);
		return _rightRotate(node);
	}
	// CASE 3
	else if(heightDiff < -1 && key < node->Right->Key)
	{
		node->Right = _rightRotate(node->Right);  
		return _leftRotate(node);
	}
	// CASE 4
	else if(heightDiff < -1 && key > node->Right->Key)
		return _leftRotate(node);

	return node;
}

void _dump(ostream& output, NODE* cur) const
{
	if(cur == nullptr)
		return;

	_dump(output, _getActualLeft(cur));

	output << "(" << cur->Key << "," << cur->Value << "," << cur->Height;
	if(cur->isThreaded && cur->Right!=nullptr)
		output << "," << cur->Right->Key << ")" << endl;
	else
		output << ")" << endl;

	_dump(output, _getActualRight(cur));
}

public:
//
// default constructor:
//
// Creates an empty tree.
//
avlt()
{
	Root = nullptr;
	Size = 0;
	Next = nullptr;
}

//
// copy constructor
//
// NOTE: makes an exact copy of the "other" tree, such that making the
// copy requires no rotations.
//
avlt (const avlt& other)
{
	Size = other.Size;
	Next = nullptr;

	NODE* threadNode;
	Root = _copy(other.Root, threadNode);
}

	//
// destructor:
//
// Called automatically by system when tree is about to be destroyed;
// this is our last chance to free any resources / memory used by
// this tree.
//
virtual ~avlt()
{
	_free(Root);
}

//
// operator=
//
// Clears "this" tree and then makes a copy of the "other" tree.
//
// NOTE: makes an exact copy of the "other" tree, such that making the
// copy requires no rotations.
//
avlt& operator=(const avlt& other)
{
	this->clear();

	NODE* threadNode;
	Root = _copy(other.Root, threadNode);
	Size = other.Size;
	Next = nullptr;

	return *this;
}

//
// clear:
//
// Clears the contents of the tree, resetting the tree to empty.
//
void clear()
{
	_free(Root);
	Root = nullptr;
	Size = 0;
	Next = nullptr;
}

// 
// size:
//
// Returns the # of nodes in the tree, 0 if empty.
//
// Time complexity:  O(1) 
//
int size() const
{
	return Size;
}

// 
// height:
//
// Returns the height of the tree, -1 if empty.
//
// Time complexity:  O(1) 
//
int height() const
{
	if (Root == nullptr)
		return -1;
	else
		return Root->Height;
}

// 
// search:
//
// Searches the tree for the given key, returning true if found
// and false if not.  If the key is found, the corresponding value
// is returned via the reference parameter.
//
// Time complexity:  O(lgN) worst-case
//
bool search(KeyT key, ValueT& value) const
{
	NODE* node = _search(Root, key);

	if(node == nullptr)
		return false;

	value = node->Value;
	return true;
}

//
// range_search
//
// Searches the tree for all keys in the range [lower..upper], inclusive.
// It is assumed that lower <= upper.  The keys are returned in a vector;
// if no keys are found, then the returned vector is empty.
//
// Time complexity: O(lgN + M), where M is the # of keys in the range
// [lower..upper], inclusive.
//
// NOTE: do not simply traverse the entire tree and select the keys
// that fall within the range.  That would be O(N), and thus invalid.
// Be smarter, you have the technology.
//
vector<KeyT> range_search(KeyT lower, KeyT upper)
{
	vector<KeyT>  keys;

	NODE *cur = Root;
	NODE *prev = nullptr;
	bool flag = false;
	while(cur!=nullptr)
	{
		if(lower == cur->Key)
		{
			flag = true;
			break;
		}

		else if(lower < cur->Key)
		{
			prev = cur;
			cur = _getActualLeft(cur);
		}

		else
		{
			prev = cur;
			cur = _getActualRight(cur);
		}
	}

	if(!flag)
		cur = prev;

	while(cur!=nullptr && cur->Key<=upper)
	{
		if(cur->Key >=lower && cur->Key<=upper)
			keys.push_back(cur->Key);

		if(cur->isThreaded)
		{
		  cur = cur->Right;
		  flag = true;
		}
		else
		{
			cur = cur->Right;
			flag = false;
			if(cur == nullptr)
				break;
			while(cur->Left != nullptr)
				cur = cur->Left;
		}
	}

	return keys;
}

//
// insert
//
// Inserts the given key into the tree; if the key has already been insert then
// the function returns without changing the tree.  Rotations are performed
// as necessary to keep the tree balanced according to AVL definition.
//
// Time complexity:  O(lgN) worst-case
//
void insert(KeyT key, ValueT value)
{
	Root = _insert(Root, key, value);
}

//
// []
//
// Returns the value for the given key; if the key is not found,
// the default value ValueT{} is returned.
//
// Time complexity:  O(lgN) worst-case
//
ValueT operator[](KeyT key) const
{
	NODE* node = _search(Root, key);

	if(node == nullptr)
		return ValueT{};

	return node->Value;
}

//
// ()
//
// Finds the key in the tree, and returns the key to the "right".
// If the right is threaded, this will be the next inorder key.
// if the right is not threaded, it will be the key of whatever
// node is immediately to the right.
//
// If no such key exists, or there is no key to the "right", the
// default key value KeyT{} is returned.
//
// Time complexity:  O(lgN) worst-case
//
KeyT operator()(KeyT key) const
{
	NODE* node = _search(Root, key);

	if(node == nullptr || node->Right == nullptr)
		return KeyT{};

	return node->Right->Key;
}

//
// %
//
// Returns the height stored in the node that contains key; if key is
// not found, -1 is returned.
//
// Example:  cout << tree%12345 << endl;
//
// Time complexity:  O(lgN) worst-case
//
int operator%(KeyT key) const
{
	NODE* node = _search(Root, key);

	if(node == nullptr)
		return -1;

	return node->Height;
}

//
// begin
//
// Resets internal state for an inorder traversal.  After the 
// call to begin(), the internal state denotes the first inorder
// key; this ensure that first call to next() function returns
// the first inorder key.
//
// Space complexity: O(1)
// Time complexity:  O(lgN) worst-case
//
// Example usage:
//    tree.begin();
//    while (tree.next(key))
//      cout << key << endl;
//
void begin()
{
	if(Root == nullptr)
		return;
	Next = Root;
	while(Next->Left != nullptr)
		Next = Next->Left;
}

//
// next
//
// Uses the internal state to return the next inorder key, and 
// then advances the internal state in anticipation of future
// calls.  If a key is in fact returned (via the reference 
// parameter), true is also returned.
//
// False is returned when the internal state has reached null,
// meaning no more keys are available.  This is the end of the
// inorder traversal.
//
// Space complexity: O(1)
// Time complexity:  O(lgN) worst-case
//
// Example usage:
//    tree.begin();
//    while (tree.next(key))
//      cout << key << endl;
//
bool next(KeyT& key)
{
	if(Next == nullptr)
		return false;

	key = Next->Key;

	if(Next->isThreaded)
		Next = Next->Right;
	else
	{
		Next = Next->Right;
		while(Next->Left != nullptr)
			Next = Next->Left;
	}
	return true;
}

//
// dump
// 
// Dumps the contents of the tree to the output stream, using a
// recursive inorder traversal.
//
void dump(ostream& output) const
{
	output << "**************************************************" << endl;
	output << "********************* AVLT ***********************" << endl;

	output << "** size: " << this->size() << endl;
	output << "** height: " << this->height() << endl;

	//
	// inorder traversal, with one output per line: either 
	// (key,value,height) or (key,value,height,THREAD)
	//
	// (key,value,height) if the node is not threaded OR thread==nullptr
	// (key,value,height,THREAD) if the node is threaded and THREAD denotes the next inorder key
	//
	_dump(output, Root);

	output << "**************************************************" << endl;
}
		
};

