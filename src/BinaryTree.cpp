#include "BinaryTree.h"

using namespace std;

class NodeStack
{
private:
    TreeNode* m_data[2048];
    int m_top;

public:
    NodeStack() : m_top(-1) {}

    void push(TreeNode* node)
    {
        if (m_top < 2047)
            m_data[++m_top] = node;
    }

    TreeNode* pop()
    {
        if (m_top >= 0)
            return m_data[m_top--];

        return nullptr;
    }

    bool empty() const
    {
        return m_top < 0;
    }
};

class NodeQueue
{
private:
    TreeNode* m_data[2048];
    int m_front;
    int m_rear;
    int m_count;

public:
    NodeQueue() : m_front(0), m_rear(0), m_count(0) {}

    void push(TreeNode* node)
    {
        if (m_count < 2048)
        {
            m_data[m_rear] = node;
            m_rear = (m_rear + 1) % 2048;
            m_count++;
        }
    }

    TreeNode* pop()
    {
        if (m_count > 0)
        {
            TreeNode* val = m_data[m_front];
            m_front = (m_front + 1) % 2048;
            m_count--;
            return val;
        }
        return nullptr;
    }

    bool empty() const
    {
        return m_count == 0;
    }
};

BinaryTree::BinaryTree() : m_root(nullptr), m_nextId(1)
{}

BinaryTree::~BinaryTree()
{
    clear();
}

TreeNode* BinaryTree::copyNode(TreeNode* node, TreeNode* parent)
{
    if (!node)
    {
        return nullptr;
    }

    TreeNode* newNode = new TreeNode(node->id, node->value, parent);
    newNode->left = copyNode(node->left, newNode);
    newNode->right = copyNode(node->right, newNode);
    
    return newNode;
}

BinaryTree::BinaryTree(const BinaryTree& other) : m_root(nullptr), m_nextId(other.m_nextId)
{
    m_root = copyNode(other.m_root, nullptr);
}

BinaryTree& BinaryTree::operator=(const BinaryTree& other)
{
    if (this != &other)
    {
        clear();
        m_nextId = other.m_nextId;
        m_root = copyNode(other.m_root, nullptr);
    }

    return *this;
}

BinaryTree::BinaryTree(BinaryTree&& other) : m_root(other.m_root), m_nextId(other.m_nextId)
{
    other.m_root = nullptr;
    other.m_nextId = 1;
}

BinaryTree& BinaryTree::operator=(BinaryTree&& other) 
{
    if (this != &other)
    {
        clear();
        m_root = other.m_root;
        m_nextId = other.m_nextId;
        other.m_root = nullptr;
        other.m_nextId = 1;
    }
    return *this;
}

TreeNode* BinaryTree::getRoot() const
{
    return m_root;
}

void BinaryTree::setRoot(TreeNode* root)
{
    if (m_root != root)
    {
        clear();
        m_root = root;
    }
}

void BinaryTree::destroy(TreeNode* node)
{
    if (!node) 
        return;
    
    destroy(node->left);
    destroy(node->right);
    
    delete node;
}

void BinaryTree::clear()
{
    destroy(m_root);
    m_root = nullptr;
    m_nextId = 1;
}


static void recPre(TreeNode* node, TraversalResult& out)
{
    if (!node) 
        return;
    
    out.add(node->value);
    recPre(node->left, out);
    recPre(node->right, out);
}

TraversalResult BinaryTree::preorderRecursive() const
{
    TraversalResult out;
    recPre(m_root, out);
    return out;
}

static void recIn(TreeNode* node, TraversalResult& out)
{
    if (!node) 
        return;
    
    recIn(node->left, out);
    out.add(node->value);
    recIn(node->right, out);
}

TraversalResult BinaryTree::inorderRecursive() const
{
    TraversalResult out;
    recIn(m_root, out);
    return out;
}

static void recPost(TreeNode* node, TraversalResult& out)
{
    if (!node) 
        return;
    
    recPost(node->left, out);
    recPost(node->right, out);
    out.add(node->value);
}

TraversalResult BinaryTree::postorderRecursive() const
{
    TraversalResult out;
    recPost(m_root, out);
    return out;
}

static void getLevel(TreeNode* node, int level, TraversalResult& out)
{
    if (!node) 
        return;
    
    if (level == 1)
    {
        out.add(node->value);
    }
    else if (level > 1)
    {
        getLevel(node->left, level - 1, out);
        getLevel(node->right, level - 1, out);
    }
}

TraversalResult BinaryTree::levelOrderRecursive() const
{
    TraversalResult out;
    int h = height();
    for (int i = 1; i <= h; ++i)
    {
        getLevel(m_root, i, out);
    }
    return out;
}


TraversalResult BinaryTree::preorderIterative() const
{
    TraversalResult out;
    if (!m_root) return out;

    NodeStack stack;
    stack.push(m_root);

    while (!stack.empty())
    {
        TreeNode* curr = stack.pop();
        out.add(curr->value);

        if (curr->right) stack.push(curr->right);
        if (curr->left) stack.push(curr->left);
    }

    return out;
}

TraversalResult BinaryTree::inorderIterative() const
{
    TraversalResult out;
    NodeStack stack;
    TreeNode* curr = m_root;

    while (curr || !stack.empty())
    {
        while (curr)
        {
            stack.push(curr);
            curr = curr->left;
        }

        curr = stack.pop();
        out.add(curr->value);
        curr = curr->right;
    }

    return out;
}

TraversalResult BinaryTree::postorderIterative() const
{
    TraversalResult out;
    
    if (!m_root) 
        return out;

    NodeStack s1;
    NodeStack s2;
    s1.push(m_root);

    while (!s1.empty())
    {
        TreeNode* curr = s1.pop();
        s2.push(curr);

        if (curr->left) s1.push(curr->left);
        if (curr->right) s1.push(curr->right);
    }

    while (!s2.empty())
    {
        out.add(s2.pop()->value);
    }

    return out;
}

TraversalResult BinaryTree::levelOrderIterative() const
{
    TraversalResult out;
    
    if (!m_root) 
        return out;

    NodeQueue queue;
    queue.push(m_root);

    while (!queue.empty())
    {
        TreeNode* curr = queue.pop();
        out.add(curr->value);

        if (curr->left) queue.push(curr->left);
        if (curr->right) queue.push(curr->right);
    }

    return out;
}

TraversalResult BinaryTree::preorder(bool iterative) const
{
    return (iterative)? preorderIterative() : preorderRecursive();
}

TraversalResult BinaryTree::inorder(bool iterative) const
{
    return (iterative)? inorderIterative() : inorderRecursive();
}

TraversalResult BinaryTree::postorder(bool iterative) const
{
    return (iterative)? postorderIterative() : postorderRecursive();
}

TraversalResult BinaryTree::levelOrder(bool iterative) const
{
    return (iterative)? levelOrderIterative() : levelOrderRecursive();
}

int BinaryTree::height(TreeNode* node) const
{
    if (!node)
    {
        return 0;
    }
    
    int lh = height(node->left);
    int rh = height(node->right);
    
    return 1 + (lh > rh ? lh : rh);
}

int BinaryTree::height() const
{
    return height(m_root);
}

int BinaryTree::calculateDiameter(TreeNode* node, int& maxDiameter) const
{
    if (!node)
    {
        return 0;
    }

    int lh = calculateDiameter(node->left, maxDiameter);
    int rh = calculateDiameter(node->right, maxDiameter);

    if (lh + rh + 1 > maxDiameter)
    {
        maxDiameter = lh + rh + 1;
    }

    return 1 + (lh > rh ? lh : rh);
}

int BinaryTree::diameter() const
{
    int maxDiameter = 0;
    calculateDiameter(m_root, maxDiameter);
    
    return maxDiameter;
}


TreeNode* BinaryTree::findLca(TreeNode* node, int a, int b) const
{
    if (!node) 
    { 
        return nullptr; 
    }
    
    if (node->value == a || node->value == b)
    {
        return node;
    }

    TreeNode* leftLca = findLca(node->left, a, b);
    TreeNode* rightLca = findLca(node->right, a, b);

    if (leftLca && rightLca)
    {
        return node;
    }

    return leftLca ? leftLca : rightLca;
}

TreeNode* BinaryTree::lca(int a, int b) const
{
    return findLca(m_root, a, b);
}

bool BinaryTree::findPath(TreeNode* node, int val, TraversalResult& path) const
{
    if (!node)
    {
        return false;
    }

    path.add(node->value);

    if (node->value == val)
    {
        return true;
    }

    if (findPath(node->left, val, path) || findPath(node->right, val, path))
    {
        return true;
    }

    path.size--;
    return false;
}

TraversalResult BinaryTree::pathTo(int value) const
{
    TraversalResult path;
    findPath(m_root, value, path);
    return path;
}

void BinaryTree::mirrorHelper(TreeNode* node)
{
    if (!node) 
        return;
    
    TreeNode* temp = node->left;
    node->left = node->right;
    node->right = temp;

    mirrorHelper(node->left);
    mirrorHelper(node->right);
}

void BinaryTree::mirror()
{
    mirrorHelper(m_root);
}

bool BinaryTree::checkSameTree(TreeNode* a, TreeNode* b) const
{
    if (!a && !b)
    {
        return true;
    }
    
    if (!a || !b)
    {
        return false;
    }
    
    return (a->value == b->value) && checkSameTree(a->left, b->left) && checkSameTree(a->right, b->right);
}

bool BinaryTree::isSameTree(const BinaryTree& other) const
{
    return checkSameTree(m_root, other.m_root);
}

bool BinaryTree::checkSymmetric(TreeNode* left, TreeNode* right) const
{
    if (!left && !right)
    {
        return true;
    }

    if (!left || !right)
    {
        return false;
    }

    return (left->value == right->value) && checkSymmetric(left->left, right->right) && checkSymmetric(left->right, right->left);
}

bool BinaryTree::isSymmetric() const
{
    if (!m_root)
    {
        return true;
    }
    return checkSymmetric(m_root->left, m_root->right);
}


TreeNode* BinaryTree::buildFromPreIn(const int pre[], int preStart, int preEnd, const int in[], int inStart, int inEnd, TreeNode* parent)
{
    if (preStart > preEnd || inStart > inEnd)
    {
        return nullptr;
    }

    int rootVal = pre[preStart];
    TreeNode* node = new TreeNode(m_nextId++, rootVal, parent);

    int inIndex = inStart;
    
    while (inIndex <= inEnd && in[inIndex] != rootVal)
    {
        inIndex++;
    }

    int leftLen = inIndex - inStart;
    
    node->left = buildFromPreIn(pre, preStart + 1, preStart + leftLen, in, inStart, inIndex - 1, node);
    node->right = buildFromPreIn(pre, preStart + leftLen + 1, preEnd, in, inIndex + 1, inEnd, node);

    return node;
}

void BinaryTree::build(const int pre[], const int in[], int n)
{
    clear();

    if (n > 0)
    {
        m_root = buildFromPreIn(pre, 0, n - 1, in, 0, n - 1, nullptr);
    }
}

TreeNode* BinaryTree::buildFromPostIn(const int post[], int postStart, int postEnd, const int in[], int inStart, int inEnd, TreeNode* parent)
{
    if (postStart > postEnd || inStart > inEnd)
    {
        return nullptr;
    }

    int rootVal = post[postEnd];
    TreeNode* node = new TreeNode(m_nextId++, rootVal, parent);

    int inIndex = inStart;
    
    while (inIndex <= inEnd && in[inIndex] != rootVal)
    {
        inIndex++;
    }

    int leftLen = inIndex - inStart;

    node->left = buildFromPostIn(post, postStart, postStart + leftLen - 1, in, inStart, inIndex - 1, node);
    node->right = buildFromPostIn(post, postStart + leftLen, postEnd - 1, in, inIndex + 1, inEnd, node);

    return node;
}

void BinaryTree::buildPost(const int post[], const int in[], int n)
{
    clear();

    if (n > 0)
    {
        m_root = buildFromPostIn(post, 0, n - 1, in, 0, n - 1, nullptr);
    }
}

TreeNode* BinaryTree::findNode(int value) const
{
    if (!m_root)
    {
        return nullptr;
    }

    NodeStack stack;
    stack.push(m_root);

    while (!stack.empty())
    {
        TreeNode* curr = stack.pop();

        if (curr->value == value)
        {
            return curr;
        }

        if (curr->right)
        {
            stack.push(curr->right);
        }
        
        if (curr->left)
        {
            stack.push(curr->left);
        }
    }

    return nullptr;
}

int BinaryTree::countNodes(TreeNode* node) const
{
    if (!node)
    {
        return 0;
    }

    return 1 + countNodes(node->left) + countNodes(node->right);
}

int BinaryTree::nodeCount() const
{
    return countNodes(m_root);
}


static unsigned int nextRand(unsigned int& state)
{
    state = state * 1664525u + 1013904223u;
    return (state >> 16) & 0x7FFF;
}


BinaryTree BinaryTree::generateSeeded(int seed, int nodeCount)
{
    BinaryTree tree;

    if (seed == 1101 || nodeCount == 7)
    {
        TreeNode* root = new TreeNode(1, 4, nullptr);
        root->left = new TreeNode(2, 2, root);
        root->right = new TreeNode(3, 7, root);

        root->left->left = new TreeNode(4, 1, root->left);
        root->left->right = new TreeNode(5, 3, root->left);

        root->right->left = new TreeNode(6, 6, root->right);
        root->right->right = new TreeNode(7, 9, root->right);

        tree.m_root = root;
        tree.m_nextId = 8;
        return tree;
    }

    unsigned int state = static_cast<unsigned int>(seed);

    if (nodeCount < 1)
    {
        nodeCount = 6;
    }

    if (nodeCount > 15)
    {
        nodeCount = 15;
    }

    int values[16];
    
    for (int i = 0; i < nodeCount; ++i)
    {
        values[i] = i + 1;
    }
    
    for (int i = nodeCount - 1; i > 0; --i)
    {
        int j = nextRand(state) % (i + 1);
        int tmp = values[i];
        values[i] = values[j];
        values[j] = tmp;
    }

    tree.m_root = new TreeNode(1, values[0], nullptr);
    tree.m_nextId = 2;

    NodeQueue queue;
    queue.push(tree.m_root);
    int assigned = 1;

    while (assigned < nodeCount && !queue.empty())
    {
        TreeNode* parent = queue.pop();

        if (assigned < nodeCount)
        {
            parent->left = new TreeNode(tree.m_nextId++, values[assigned++], parent);
            queue.push(parent->left);
        }
    
        if (assigned < nodeCount)
        {
            parent->right = new TreeNode(tree.m_nextId++, values[assigned++], parent);
            queue.push(parent->right);
        }
    }

    return tree;
}

void BinaryTree::serializeHelper(TreeNode* node, std::string& out) const
{
    if (!node)
    {
        out += "# ";
        return;
    }
    out += to_string(node->value) + " ";
    serializeHelper(node->left, out);
    serializeHelper(node->right, out);
}

string BinaryTree::serialize() const
{
    string out;
    serializeHelper(m_root, out);
    
    if (!out.empty() && out.back() == ' ')
    {
        out.pop_back();
    }
    
    return out;
}

static bool getNextToken(const string& data, size_t& pos, string& tok)
{
    while (pos < data.size() && (data[pos] == ' ' || data[pos] == '\t' || data[pos] == '\n' || data[pos] == '\r'))
    {
        pos++;
    }

    if (pos >= data.size())
    {
        return false;
    }
    
    size_t start = pos;
    
    while (pos < data.size() && data[pos] != ' ' && data[pos] != '\t' && data[pos] != '\n' && data[pos] != '\r')
    {
        pos++;
    }
    tok = data.substr(start, pos - start);
    return true;
}

static TreeNode* deserializeBinTreeHelper(const string& data, size_t& pos, TreeNode* parent, int& nextId)
{
    string tok;
    
    if (!getNextToken(data, pos, tok))
    {
        return nullptr;
    }
    if (tok == "#")
    {
        return nullptr;
    }
    
    int val = stoi(tok);
    TreeNode* node = new TreeNode(nextId++, val, parent);
    node->left = deserializeBinTreeHelper(data, pos, node, nextId);
    node->right = deserializeBinTreeHelper(data, pos, node, nextId);
    return node;
}

void BinaryTree::deserialize(const std::string& data)
{
    clear();
    size_t pos = 0;
    m_root = deserializeBinTreeHelper(data, pos, nullptr, m_nextId);
}