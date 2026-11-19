#pragma once

// TreeNode definition for Act 1 and Act 2 (Section 5.1)
struct TreeNode
{
    int id;
    int value;
    TreeNode* left;
    TreeNode* right;
    TreeNode* parent;

    TreeNode(int id = 0, int value = 0, TreeNode* parent = nullptr)
        : id(id), value(value), left(nullptr), right(nullptr), parent(parent)
    {
    }
};

// Fixed-capacity traversal result (no vector, no templates)
struct TraversalResult
{
    int data[32];
    int size = 0;

    void add(int val)
    {
        if (size < 32)
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

// BinaryTree implementation for Act 1 (Section 5.2)
// Pure data structure - no SFML, no vector, no map, no algorithm, no templates
class BinaryTree
{
public:
    BinaryTree();
    ~BinaryTree();
    BinaryTree(const BinaryTree& other);
    BinaryTree& operator=(const BinaryTree& other);
    BinaryTree(BinaryTree&& other) noexcept;
    BinaryTree& operator=(BinaryTree&& other) noexcept;

    TreeNode* getRoot() const;
    void setRoot(TreeNode* root);
    void clear();

    // Traversals: recursive and iterative (Section 5.2)
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

    // Reconstruction (Section 5.2)
    void build(const int pre[], const int in[], int n);
    void buildPost(const int post[], const int in[], int n);

    // Height & Diameter
    // Convention: Leaf node has height = 1, nullptr has height = 0
    int height(TreeNode* node) const;
    int height() const;
    int diameter() const;

    // LCA & Path
    TreeNode* lca(int a, int b) const;
    TraversalResult pathTo(int value) const;

    // Mirror & Checks
    void mirror();
    bool isSameTree(const BinaryTree& other) const;
    bool isSymmetric() const;

    // Seeded tree generator
    static BinaryTree generateSeeded(int seed, int nodeCount = 7);

    TreeNode* findNode(int value) const;
    int nodeCount() const;

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

    TreeNode* buildFromPreIn(const int pre[], int preStart, int preEnd,
                             const int in[], int inStart, int inEnd,
                             TreeNode* parent);
    TreeNode* buildFromPostIn(const int post[], int postStart, int postEnd,
                              const int in[], int inStart, int inEnd,
                              TreeNode* parent);
    int countNodes(TreeNode* node) const;
};
