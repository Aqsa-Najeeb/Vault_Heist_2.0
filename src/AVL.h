#pragma once
#include "BinaryTree.h"
#include "EventLog.h"
#include <string>
using namespace std;

struct AVLNode
{
    int id;
    int value;
    AVLNode* left;
    AVLNode* right;
    AVLNode* parent;
    int height;
    int size;

    AVLNode(int id = 0, int value = 0, AVLNode* parent = nullptr);
};

class AVLTree
{
public:
    AVLTree();
    ~AVLTree();
    AVLTree(const AVLTree& other);
    AVLTree& operator=(const AVLTree& other);

    AVLNode* getRoot() const;
    void setRoot(AVLNode* root);
    void clear();

    bool insert(int val, EventLog* log = nullptr);
    bool deleteKey(int val, EventLog* log = nullptr);
    AVLNode* search(int val) const;

    AVLNode* rotateLeft(AVLNode* node, EventLog* log = nullptr);
    AVLNode* rotateRight(AVLNode* node, EventLog* log = nullptr);
    AVLNode* rebalance(AVLNode* node, EventLog* log = nullptr);

    int height(AVLNode* node) const;
    int size(AVLNode* node) const;
    int balanceFactor(AVLNode* node) const;

    int kth(int k) const;
    int rank(int val) const;
    int countInRange(int lo, int hi) const;

    bool isBalanced() const;
    int totalNodes() const;
    int treeHeight() const;

    TreeNode* toBinaryTree() const;
    static void freeBinaryTree(TreeNode* node);

    string serialize() const;
    void deserialize(const std::string& data);

private:
    AVLNode* m_root;
    int m_nextId;

    void destroy(AVLNode* node);
    AVLNode* copyNode(AVLNode* node, AVLNode* parent);
    void updateNode(AVLNode* node);
    AVLNode* insertHelper(AVLNode* node, AVLNode* parent, int val, bool& ok, EventLog* log);
    AVLNode* deleteHelper(AVLNode* node, int val, bool& ok, EventLog* log);
    AVLNode* findMin(AVLNode* node) const;
    bool checkBalanced(AVLNode* node) const;
    int kthHelper(AVLNode* node, int k) const;
    int rankHelper(AVLNode* node, int val) const;
    TreeNode* convertHelper(AVLNode* node, TreeNode* parent) const;
    void serializeHelper(AVLNode* node, std::string& out) const;
};
