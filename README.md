# Threaded-AVL-Tree
Threaded AVL: alt<KeyT, ValueT>

-This program uses treaded AVL tree and performs inorder traversal.
-Time complexity of AVL tree search and insert is O(lgN) 
-Inorder traversal is performed in O(N) using O(1) space
-insert() function updates the heights and rebalances the tree as necessary
-Left and right rotations are made to rebalance the tree whenever needed.
-height() function returns the height of the trey retrieving the height from the root node; O(1) time complexity.
-operator%() function returns the height stored in the given node
- range_search() function searches theatre for all keys in the range[lower..upper], keys are returned in a vector and if no keys are found then empty vector is returned
-dump() function dumps the content of the tree to output stream, using a recursive inorder traversal
