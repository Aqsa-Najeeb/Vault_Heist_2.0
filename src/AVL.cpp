#include "AVL.h"
using namespace std;

AVLNode::AVLNode(int id, int value, AVLNode* parent) : id(id), value(value), left(nullptr), right(nullptr), parent(parent), height(1), size(1)
{}

AVLTree::AVLTree() : m_root(nullptr), m_nextId(1)
{}

AVLTree::~AVLTree()
{
    clear();
}

void AVLTree::destroy(AVLNode* node)
{
    if (!node)
    {
        return;
    }

    destroy(node->left);
    destroy(node->right);

    delete node;
}

AVLNode* AVLTree::copyNode(AVLNode* node, AVLNode* parent)
{
    if (!node)
    {
        return nullptr;
    }
    AVLNode* newNode = new AVLNode(node->id, node->value, parent);
    newNode->height = node->height;
    newNode->size = node->size;
    newNode->left = copyNode(node->left, newNode);
    newNode->right = copyNode(node->right, newNode);

    return newNode;
}

AVLTree::AVLTree(const AVLTree& other) : m_root(nullptr), m_nextId(other.m_nextId)
{
    m_root = copyNode(other.m_root, nullptr);
}

AVLTree& AVLTree::operator=(const AVLTree& other)
{
    if (this != &other)
    {
        clear();
        m_nextId = other.m_nextId;
        m_root = copyNode(other.m_root, nullptr);
    }
    return *this;
}

AVLNode* AVLTree::getRoot() const
{
    return m_root;
}

void AVLTree::setRoot(AVLNode* root)
{
    m_root = root;
}

void AVLTree::clear()
{
    destroy(m_root);
    m_root = nullptr;
    m_nextId = 1;
}

int AVLTree::height(AVLNode* node) const
{
    return (node ? node->height : 0);
}

int AVLTree::size(AVLNode* node) const
{
    return (node ? node->size : 0);
}

int AVLTree::balanceFactor(AVLNode* node) const
{
    return (node ? (height(node->left) - height(node->right)) : 0);
}

void AVLTree::updateNode(AVLNode* node)
{
    if (!node)
    {
        return;
    }
    int lh = height(node->left);
    int rh = height(node->right);
    node->height = 1 + (lh > rh ? lh : rh);
    node->size = 1 + size(node->left) + size(node->right);
}

AVLNode* AVLTree::rotateRight(AVLNode* y, EventLog* log)
{
    AVLNode* x = y->left;
    if (!x)
    {
        return y;
    }
    AVLNode* T2 = x->right;
    AVLNode* p = y->parent;

    x->right = y;
    y->parent = x;
    y->left = T2;
    if (T2)
    {
        T2->parent = y;
    }
    x->parent = p;

    if (!p)
    {
        m_root = x;
    }
    else if (p->left == y)
    {
        p->left = x;
    }
    else
    {
        p->right = x;
    }

    updateNode(y);
    updateNode(x);

    if (log)
    {
        log->record(EventType::ROTATE, "[ROTATE] RR (Right) Rotation on pivot " + to_string(y->value));
    }
    return x;
}

AVLNode* AVLTree::rotateLeft(AVLNode* x, EventLog* log)
{
    AVLNode* y = x->right;
    if (!y)
    {
        return x;
    }
    AVLNode* T2 = y->left;
    AVLNode* p = x->parent;

    y->left = x;
    x->parent = y;
    x->right = T2;
    if (T2)
    {
        T2->parent = x;
    }
    y->parent = p;

    if (!p)
    {
        m_root = y;
    }
    else if (p->left == x)
    {
        p->left = y;
    }
    else
    {
        p->right = y;
    }

    updateNode(x);
    updateNode(y);

    if (log)
    {
        log->record(EventType::ROTATE, "[ROTATE] LL (Left) Rotation on pivot " + to_string(x->value));
    }
    return y;
}

AVLNode* AVLTree::rebalance(AVLNode* node, EventLog* log)
{
    if (!node)
    {
        return nullptr;
    }
    updateNode(node);
    int bf = balanceFactor(node);

    if (bf > 1)
    {
        if (balanceFactor(node->left) < 0)
        {
            if (log)
            {
                log->record(EventType::ROTATE, "[ROTATE] LR Double Rotation on " + to_string(node->value));
            }

            node->left = rotateLeft(node->left, log);

            if (node->left)
            {
                node->left->parent = node;
            }
        }
        return rotateRight(node, log);
    }
    if (bf < -1)
    {
        if (balanceFactor(node->right) > 0)
        {
            if (log)
            {
                log->record(EventType::ROTATE, "[ROTATE] RL Double Rotation on " + to_string(node->value));
            }

            node->right = rotateRight(node->right, log);
            
            if (node->right)
            {
                node->right->parent = node;
            }
        }
        return rotateLeft(node, log);
    }
    return node;
}

AVLNode* AVLTree::insertHelper(AVLNode* node, AVLNode* parent, int val, bool& ok, EventLog* log)
{
    if (!node)
    {
        ok = true;
        AVLNode* newNode = new AVLNode(m_nextId++, val, parent);
        if (log)
        {
            log->record(EventType::INSERT, "[INSERT] Node " + to_string(val) + " inserted");
        }
        return newNode;
    }

    if (val == node->value)
    {
        ok = false;
        if (log)
        {
            log->record(EventType::COMPARE, "[REJECT] Duplicate key " + to_string(val));
        }
        return node;
    }

    if (val < node->value)
    {
        node->left = insertHelper(node->left, node, val, ok, log);
        if (node->left)
        {
            node->left->parent = node;
        }
    }
    else
    {
        node->right = insertHelper(node->right, node, val, ok, log);
        if (node->right)
        {
            node->right->parent = node;
        }
    }

    if (!ok)
    {
        return node;
    }

    return rebalance(node, log);
}

bool AVLTree::insert(int val, EventLog* log)
{
    bool ok = false;
    m_root = insertHelper(m_root, nullptr, val, ok, log);
    if (m_root)
    {
        m_root->parent = nullptr;
    }
    if (log && ok)
    {
        log->record(EventType::DONE, "[DONE] Insert complete");
    }
    return ok;
}

AVLNode* AVLTree::findMin(AVLNode* node) const
{
    if (!node)
    {
        return nullptr;
    }
    while (node->left)
    {
        node = node->left;
    }
    return node;
}

AVLNode* AVLTree::deleteHelper(AVLNode* node, int val, bool& ok, EventLog* log)
{
    if (!node)
    {
        ok = false;
        return nullptr;
    }

    if (val < node->value)
    {
        node->left = deleteHelper(node->left, val, ok, log);
        if (node->left)
        {
            node->left->parent = node;
        }
    }
    else if (val > node->value)
    {
        node->right = deleteHelper(node->right, val, ok, log);
        if (node->right)
        {
            node->right->parent = node;
        }
    }
    else
    {
        ok = true;
        if (!node->left || !node->right)
        {
            AVLNode* temp = node->left ? node->left : node->right;
            if (!temp)
            {
                if (log)
                {
                    log->record(EventType::UNLINK, "[DELETE] Key " + to_string(val) + " unlinked");
                }
                delete node;
                return nullptr;
            }
            else
            {
                temp->parent = node->parent;
                if (log)
                {
                    log->record(EventType::UNLINK, "[DELETE] Key " + to_string(val) + " bypassed");
                }
                delete node;
                node = temp;
            }
        }
        else
        {
            AVLNode* succ = findMin(node->right);
            node->value = succ->value;
            if (log)
            {
                log->record(EventType::SWAP_VALUE, "[DELETE] Swapped with successor " + to_string(succ->value));
            }
            node->right = deleteHelper(node->right, succ->value, ok, log);
            if (node->right)
            {
                node->right->parent = node;
            }
        }
    }

    if (!node)
    {
        return nullptr;
    }

    return rebalance(node, log);
}

bool AVLTree::deleteKey(int val, EventLog* log)
{
    bool ok = false;
    m_root = deleteHelper(m_root, val, ok, log);
    if (m_root)
    {
        m_root->parent = nullptr;
    }
    if (log)
    {
        if (ok)
        {
            log->record(EventType::DONE, "[DONE] Delete complete");
        }
        else
        {
            log->record(EventType::DONE, "[DELETE] Key " + to_string(val) + " not found");
        }
    }
    return ok;
}

AVLNode* AVLTree::search(int val) const
{
    AVLNode* curr = m_root;
    while (curr)
    {
        if (curr->value == val)
        {
            return curr;
        }
        if (val < curr->value)
        {
            curr = curr->left;
        }
        else
        {
            curr = curr->right;
        }
    }
    return nullptr;
}

int AVLTree::kthHelper(AVLNode* node, int k) const
{
    if (!node)
    {
        return -1;
    }
    int leftSize = size(node->left);
    if (k == leftSize + 1)
    {
        return node->value;
    }
    if (k <= leftSize)
    {
        return kthHelper(node->left, k);
    }
    return kthHelper(node->right, k - leftSize - 1);
}

int AVLTree::kth(int k) const
{
    return kthHelper(m_root, k);
}

int AVLTree::rankHelper(AVLNode* node, int val) const
{
    if (!node)
    {
        return 0;
    }
    if (val < node->value)
    {
        return rankHelper(node->left, val);
    }
    if (val == node->value)
    {
        return size(node->left) + 1;
    }
    return size(node->left) + 1 + rankHelper(node->right, val);
}

int AVLTree::rank(int val) const
{
    return rankHelper(m_root, val);
}

int AVLTree::countInRange(int lo, int hi) const
{
    return rank(hi) - rank(lo - 1);
}

bool AVLTree::checkBalanced(AVLNode* node) const
{
    if (!node)
    {
        return true;
    }
    int bf = balanceFactor(node);
    if (bf < -1 || bf > 1)
    {
        return false;
    }
    return checkBalanced(node->left) && checkBalanced(node->right);
}

bool AVLTree::isBalanced() const
{
    return checkBalanced(m_root);
}

int AVLTree::totalNodes() const
{
    return size(m_root);
}

int AVLTree::treeHeight() const
{
    return height(m_root);
}

TreeNode* AVLTree::convertHelper(AVLNode* node, TreeNode* parent) const
{
    if (!node)
    {
        return nullptr;
    }
    TreeNode* tn = new TreeNode(node->id, node->value, parent);
    tn->left = convertHelper(node->left, tn);
    tn->right = convertHelper(node->right, tn);
    
    return tn;
}

TreeNode* AVLTree::toBinaryTree() const
{
    return convertHelper(m_root, nullptr);
}

void AVLTree::freeBinaryTree(TreeNode* node)
{
    if (!node)
    {
        return;
    }
    
    freeBinaryTree(node->left);
    freeBinaryTree(node->right);
    
    delete node;
}

void AVLTree::serializeHelper(AVLNode* node, string& out) const
{
    if (!node)
    {
        out += "# ";
        return;
    }
    out += std::to_string(node->value) + " ";
    serializeHelper(node->left, out);
    serializeHelper(node->right, out);
}

string AVLTree::serialize() const
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

static AVLNode* deserializeAVLHelper(const string& data, size_t& pos, AVLNode* parent, int& nextId)
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
    AVLNode* node = new AVLNode(nextId++, val, parent);
    node->left = deserializeAVLHelper(data, pos, node, nextId);
    node->right = deserializeAVLHelper(data, pos, node, nextId);
    int lh = node->left ? node->left->height : 0;
    int rh = node->right ? node->right->height : 0;
    node->height = 1 + (lh > rh ? lh : rh);
    int ls = node->left ? node->left->size : 0;
    int rs = node->right ? node->right->size : 0;
    node->size = 1 + ls + rs;

    return node;
}

void AVLTree::deserialize(const string& data)
{
    clear();
    size_t pos = 0;
    m_root = deserializeAVLHelper(data, pos, nullptr, m_nextId);
}
