#pragma once
#include <string>
using namespace std;

struct TreeNode
{
    int id;
    int value;
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;

    TreeNode(int id = 0, int value = 0, TreeNode* parent = nullptr) : id(id), value(value), left(nullptr), right(nullptr), parent(parent)
    {}
};

struct TraversalResult
{
    int data[2048];
    int size = 0;

    void add(int val)
    {
        if (size < 2048)
        {
            data[size++] = val;
        }
    }

    int get(int index) const
    {
        if (index >= 0 && index < size)
            return data[index];
        return 0;
    }
};

class BinaryTree
{
public:
    BinaryTree();
    ~BinaryTree();
    BinaryTree(const BinaryTree& other);
    BinaryTree& operator=(const BinaryTree& other);
    BinaryTree(BinaryTree&& other);
    BinaryTree& operator=(BinaryTree&& other);

    TreeNode* getRoot() const;
    void setRoot(TreeNode* root);
    void clear();

    TraversalResult preorder(bool iterative = false) const;
    TraversalResult inorder(bool iterative = false) const;
    TraversalResult postorder(bool iterative = false) const;
    TraversalResult levelOrder(bool iterative = false) const;

    TraversalResult preorderRecursive() const;
    TraversalResult preorderIterative() const;

    TraversalResult inorderRecursive() const;
    TraversalResult inorderIterative() const;

    TraversalResult postorderRecursive() const;
    TraversalResult postorderIterative() const;

    TraversalResult levelOrderRecursive() const;
    TraversalResult levelOrderIterative() const;

    void build(const int pre[], const int in[], int n);
    void buildPost(const int post[], const int in[], int n);

    int height(TreeNode* node) const;
    int height() const;
    int diameter() const;

    TreeNode* lca(int a, int b) const;
    TraversalResult pathTo(int value) const;

    void mirror();
    bool isSameTree(const BinaryTree& other) const;
    bool isSymmetric() const;

    static BinaryTree generateSeeded(int seed, int nodeCount = 7);

    TreeNode* findNode(int value) const;
    int nodeCount() const;

    string serialize() const;
    void deserialize(const string& data);

private:
    TreeNode* m_root;
    int m_nextId;

    TreeNode* copyNode(TreeNode* node, TreeNode* parent);
    void destroy(TreeNode* node);
    int calculateDiameter(TreeNode* node, int& maxDiameter) const;
    TreeNode* findLca(TreeNode* node, int a, int b) const;
    bool findPath(TreeNode* node, int val, TraversalResult& path) const;
    void mirrorHelper(TreeNode* node);
    bool checkSameTree(TreeNode* a, TreeNode* b) const;
    bool checkSymmetric(TreeNode* left, TreeNode* right) const;

    TreeNode* buildFromPreIn(const int pre[], int preStart, int preEnd, const int in[], int inStart, int inEnd, TreeNode* parent);
    TreeNode* buildFromPostIn(const int post[], int postStart, int postEnd, const int in[], int inStart, int inEnd, TreeNode* parent);
    int countNodes(TreeNode* node) const;
    void serializeHelper(TreeNode* node, string& out) const;
};
