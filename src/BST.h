#pragma once
#include "BinaryTree.h"
#include "EventLog.h"
#include <string>
using namespace std;

class BST
{
public:
    BST();
    ~BST();
    BST(const BST& other);
    BST& operator=(const BST& other);

    TreeNode* getRoot() const;
    void setRoot(TreeNode* root);
    void clear();

    bool insert(int val, EventLog* log = nullptr);
    TreeNode* search(int val, EventLog* log = nullptr) const;
    bool deleteKey(int val, EventLog* log = nullptr);

    TreeNode* findMin(TreeNode* node) const;
    TreeNode* findMax(TreeNode* node) const;
    TreeNode* successor(TreeNode* node) const;
    TreeNode* predecessor(TreeNode* node) const;

    void rangeQuery(int lo, int hi, int* const out, int& count, EventLog* log = nullptr) const;
    int kthSmallest(int k) const;
    TreeNode* lcaBST(int a, int b) const;

    int size() const;
    int height() const;

    string serialize() const;
    void deserialize(const string& data);

private:
    TreeNode* m_root;
    int m_nextId;

    void destroy(TreeNode* node);
    TreeNode* copyNode(TreeNode* node, TreeNode* parent);
    int countNodes(TreeNode* node) const;
    int calcHeight(TreeNode* node) const;
    void rangeHelper(TreeNode* node, int lo, int hi, int* const out, int& count, EventLog* log) const;
    void inorderKth(TreeNode* node, int& count, int k, int& result) const;
    void serializeHelper(TreeNode* node, string& out) const;
};
