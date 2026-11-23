#include "Validator.h"

bool Validator::checkParentLinks(const TreeNode* root)
{
    if (!root)
    {
        return true;
    }
    if (root->left)
    {
        if (root->left->parent != root)
        {
            return false;
        }
        if (!checkParentLinks(root->left))
        {
            return false;
        }
    }
    if (root->right)
    {
        if (root->right->parent != root)
        {
            return false;
        }
        if (!checkParentLinks(root->right))
        {
            return false;
        }
    }
    return true;
}

bool Validator::checkAVLParentLinks(const AVLNode* root)
{
    if (!root)
    {
        return true;
    }
    if (root->left)
    {
        if (root->left->parent != root)
        {
            return false;
        }
        if (!checkAVLParentLinks(root->left))
        {
            return false;
        }
    }
    if (root->right)
    {
        if (root->right->parent != root)
        {
            return false;
        }
        if (!checkAVLParentLinks(root->right))
        {
            return false;
        }
    }
    return true;
}

bool Validator::bstHelper(const TreeNode* node, int minVal, int maxVal)
{
    if (!node)
    {
        return true;
    }
    if (node->value <= minVal || node->value >= maxVal)
    {
        return false;
    }
    return bstHelper(node->left, minVal, node->value) && bstHelper(node->right, node->value, maxVal);
}

static bool checkNoCyclesBST(const TreeNode* root, const TreeNode* visited[], int& count, int maxCount)
{
    if (!root)
    {
        return true;
    }
    for (int i = 0; i < count; ++i)
    {
        if (visited[i] == root)
        {
            return false;
        }
    }
    if (count < maxCount)
    {
        visited[count++] = root;
    }
    else
    {
        return false;
    }
    return checkNoCyclesBST(root->left, visited, count, maxCount) && checkNoCyclesBST(root->right, visited, count, maxCount);
}

bool Validator::validateBST(const TreeNode* root)
{
    if (!root)
    {
        return true;
    }
    if (root->parent != nullptr)
    {
        return false;
    }
    if (!checkParentLinks(root))
    {
        return false;
    }
    if (!bstHelper(root, -2147483648, 2147483647))
    {
        return false;
    }

    const TreeNode* visited[2048];
    int count = 0;
    if (!checkNoCyclesBST(root, visited, count, 2048))
    {
        return false;
    }

    return true;
}

int Validator::avlHelper(const AVLNode* node, bool& isBal)
{
    if (!node)
    {
        return 0;
    }

    int lh = avlHelper(node->left, isBal);
    int rh = avlHelper(node->right, isBal);

    int computedHeight = 1 + (lh > rh ? lh : rh);
    if (node->height != computedHeight)
    {
        isBal = false;
    }

    int ls = node->left ? node->left->size : 0;
    int rs = node->right ? node->right->size : 0;
    if (node->size != 1 + ls + rs)
    {
        isBal = false;
    }

    int bf = lh - rh;
    if (bf < -1 || bf > 1)
    {
        isBal = false;
    }

    return computedHeight;
}

static bool checkAVLOrdering(const AVLNode* node, int minVal, int maxVal)
{
    if (!node)
    {
        return true;
    }
    if (node->value <= minVal || node->value >= maxVal)
    {
        return false;
    }
    return checkAVLOrdering(node->left, minVal, node->value) && checkAVLOrdering(node->right, node->value, maxVal);
}

static bool checkNoCyclesAVL(const AVLNode* root, const AVLNode* visited[], int& count, int maxCount)
{
    if (!root)
    {
        return true;
    }
    for (int i = 0; i < count; ++i)
    {
        if (visited[i] == root)
        {
            return false;
        }
    }
    if (count < maxCount)
    {
        visited[count++] = root;
    }
    else
    {
        return false;
    }
    return checkNoCyclesAVL(root->left, visited, count, maxCount) && checkNoCyclesAVL(root->right, visited, count, maxCount);
}

bool Validator::validateAVL(const AVLNode* root)
{
    if (!root)
    {
        return true;
    }
    if (root->parent != nullptr)
    {
        return false;
    }
    if (!checkAVLParentLinks(root))
    {
        return false;
    }
    if (!checkAVLOrdering(root, -2147483648, 2147483647))
    {
        return false;
    }

    bool isBal = true;
    avlHelper(root, isBal);
    if (!isBal)
    {
        return false;
    }

    const AVLNode* visited[2048];
    int count = 0;
    if (!checkNoCyclesAVL(root, visited, count, 2048))
    {
        return false;
    }

    return true;
}

bool Validator::validateMaxHeap(const int* arr, int n)
{
    if (!arr || n <= 1)
    {
        return true;
    }
    for (int i = 0; i <= (n - 2) / 2; ++i)
    {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < n && arr[i] < arr[l])
        {
            return false;
        }
        if (r < n && arr[i] < arr[r])
        {
            return false;
        }
    }
    return true;
}
