#pragma once
#include "BinaryTree.h"
#include "AVL.h"

class Validator
{
public:
    static bool validateBST(const TreeNode* root);
    static bool validateAVL(const AVLNode* root);
    static bool validateMaxHeap(const int* arr, int n);
    static bool checkParentLinks(const TreeNode* root);
    static bool checkAVLParentLinks(const AVLNode* root);

private:
    static bool bstHelper(const TreeNode* node, int minVal, int maxVal);
    static int avlHelper(const AVLNode* node, bool& isBal);
};
