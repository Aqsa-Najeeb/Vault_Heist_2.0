#include "BST.h"
using namespace std;

BST::BST() : m_root(nullptr), m_nextId(1)
{}

BST::~BST()
{
    clear();
}

void BST::destroy(TreeNode* node)
{
    if (!node)
    {
        return;
    }
    destroy(node->left);
    destroy(node->right);
    delete node;
}

TreeNode* BST::copyNode(TreeNode* node, TreeNode* parent)
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

BST::BST(const BST& other) : m_root(nullptr), m_nextId(other.m_nextId)
{
    m_root = copyNode(other.m_root, nullptr);
}

BST& BST::operator=(const BST& other)
{
    if (this != &other)
    {
        clear();
        m_nextId = other.m_nextId;
        m_root = copyNode(other.m_root, nullptr);
    }
    return *this;
}

TreeNode* BST::getRoot() const
{
    return m_root;
}

void BST::setRoot(TreeNode* root)
{
    m_root = root;
}

void BST::clear()
{
    destroy(m_root);
    m_root = nullptr;
    m_nextId = 1;
}

bool BST::insert(int val, EventLog* log)
{
    if (!m_root)
    {
        m_root = new TreeNode(m_nextId++, val, nullptr);
        if (log)
        {
            log->record(EventType::INSERT, "[INSERT] Created root node " + to_string(val));
            log->record(EventType::DONE, "[DONE] Insert complete");
        }
        return true;
    }

    TreeNode* curr = m_root;
    TreeNode* parent = nullptr;
    bool goLeft = false;

    while (curr)
    {
        parent = curr;
        if (log)
        {
            log->record(EventType::COMPARE, "[COMPARE] Comparing " + to_string(val) + " with " + to_string(curr->value));
        }
        if (val == curr->value)
        {
            if (log)
            {
                log->record(EventType::COMPARE, "[REJECT] Duplicate key " + to_string(val));
                log->record(EventType::DONE, "[DONE] Insert rejected");
            }
            return false;
        }
        if (val < curr->value)
        {
            goLeft = true;
            curr = curr->left;
        }
        else
        {
            goLeft = false;
            curr = curr->right;
        }
    }

    TreeNode* newNode = new TreeNode(m_nextId++, val, parent);
    if (goLeft)
    {
        parent->left = newNode;
    }
    else
    {
        parent->right = newNode;
    }

    if (log)
    {
        log->record(EventType::INSERT, "[INSERT] Node " + to_string(val) + " inserted");
        log->record(EventType::DONE, "[DONE] Insert complete");
    }
    return true;
}

TreeNode* BST::search(int val, EventLog* log) const
{
    TreeNode* curr = m_root;
    while (curr)
    {
        if (log)
        {
            log->record(EventType::COMPARE, "[COMPARE] Comparing " + to_string(val) + " with " + to_string(curr->value));
        }
        if (curr->value == val)
        {
            if (log)
            {
                log->record(EventType::DONE, "[SEARCH] Found target node " + to_string(val));
            }
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
    if (log)
    {
        log->record(EventType::DONE, "[SEARCH] Key " + to_string(val) + " not found");
    }
    return nullptr;
}

bool BST::deleteKey(int val, EventLog* log)
{
    TreeNode* curr = m_root;
    while (curr && curr->value != val)
    {
        if (log)
        {
            log->record(EventType::COMPARE, "[COMPARE] Searching for " + to_string(val) + " at " + to_string(curr->value));
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

    if (!curr)
    {
        if (log)
        {
            log->record(EventType::DONE, "[DELETE] Key " + to_string(val) + " not found");
        }
        return false;
    }

    if (!curr->left && !curr->right)
    {
        if (log)
        {
            log->record(EventType::UNLINK, "[DELETE] Key " + to_string(val) + " unlinked (leaf)");
        }
        if (curr == m_root)
        {
            m_root = nullptr;
        }
        else
        {
            if (curr->parent->left == curr)
            {
                curr->parent->left = nullptr;
            }
            else
            {
                curr->parent->right = nullptr;
            }
        }
        delete curr;
    }
    else if (!curr->left || !curr->right)
    {
        TreeNode* child = curr->left ? curr->left : curr->right;
        if (log)
        {
            log->record(EventType::UNLINK, "[DELETE] Key " + to_string(val) + " bypassed (one child)");
        }
        if (curr == m_root)
        {
            m_root = child;
            child->parent = nullptr;
        }
        else
        {
            if (curr->parent->left == curr)
            {
                curr->parent->left = child;
            }
            else
            {
                curr->parent->right = child;
            }
            child->parent = curr->parent;
        }
        delete curr;
    }
    else
    {
        TreeNode* succ = findMin(curr->right);
        int succVal = succ->value;
        if (log)
        {
            log->record(EventType::SWAP_VALUE, "[DELETE] Key " + to_string(val) + " swapped with successor " + to_string(succVal));
        }
        curr->value = succVal;

        TreeNode* succChild = succ->right;
        if (succ->parent->left == succ)
        {
            succ->parent->left = succChild;
        }
        else
        {
            succ->parent->right = succChild;
        }
        if (succChild)
        {
            succChild->parent = succ->parent;
        }
        if (log)
        {
            log->record(EventType::UNLINK, "[DELETE] Successor node unlinked");
        }
        delete succ;
    }

    if (log)
    {
        log->record(EventType::DONE, "[DONE] Delete complete");
    }
    return true;
}

TreeNode* BST::findMin(TreeNode* node) const
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

TreeNode* BST::findMax(TreeNode* node) const
{
    if (!node)
    {
        return nullptr;
    }
    while (node->right)
    {
        node = node->right;
    }
    return node;
}

TreeNode* BST::successor(TreeNode* node) const
{
    if (!node)
    {
        return nullptr;
    }
    if (node->right)
    {
        return findMin(node->right);
    }
    TreeNode* p = node->parent;
    while (p && node == p->right)
    {
        node = p;
        p = p->parent;
    }
    return p;
}

TreeNode* BST::predecessor(TreeNode* node) const
{
    if (!node)
    {
        return nullptr;
    }
    if (node->left)
    {
        return findMax(node->left);
    }
    TreeNode* p = node->parent;
    while (p && node == p->left)
    {
        node = p;
        p = p->parent;
    }
    return p;
}

void BST::rangeHelper(TreeNode* node, int lo, int hi, int* const out, int& count, EventLog* log) const
{
    if (!node)
    {
        return;
    }
    if (node->value > lo)
    {
        rangeHelper(node->left, lo, hi, out, count, log);
    }
    else if (log && node->left)
    {
        log->record(EventType::HIGHLIGHT_SUBTREE, "[PRUNE] Left subtree of " + to_string(node->value) + " skipped");
    }

    if (node->value >= lo && node->value <= hi)
    {
        out[count++] = node->value;
        if (log)
        {
            log->record(EventType::VISIT, "[RANGE] Included " + to_string(node->value));
        }
    }

    if (node->value < hi)
    {
        rangeHelper(node->right, lo, hi, out, count, log);
    }
    else if (log && node->right)
    {
        log->record(EventType::HIGHLIGHT_SUBTREE, "[PRUNE] Right subtree of " + to_string(node->value) + " skipped");
    }
}

void BST::rangeQuery(int lo, int hi, int* const out, int& count, EventLog* log) const
{
    count = 0;
    rangeHelper(m_root, lo, hi, out, count, log);
    if (log)
    {
        log->record(EventType::DONE, "[DONE] Range query complete");
    }
}

void BST::inorderKth(TreeNode* node, int& count, int k, int& result) const
{
    if (!node || result != -1)
    {
        return;
    }

    inorderKth(node->left, count, k, result);
    count++;
    
    if (count == k)
    {
        result = node->value;
        return;
    }
    inorderKth(node->right, count, k, result);
}

int BST::kthSmallest(int k) const
{
    int count = 0;
    int result = -1;
    inorderKth(m_root, count, k, result);
    return result;
}

TreeNode* BST::lcaBST(int a, int b) const
{
    TreeNode* curr = m_root;
    while (curr)
    {
        if (a < curr->value && b < curr->value)
        {
            curr = curr->left;
        }
        else if (a > curr->value && b > curr->value)
        {
            curr = curr->right;
        }
        else
        {
            return curr;
        }
    }
    return nullptr;
}

int BST::countNodes(TreeNode* node) const
{
    if (!node)
    {
        return 0;
    }
    return 1 + countNodes(node->left) + countNodes(node->right);
}

int BST::size() const
{
    return countNodes(m_root);
}

int BST::calcHeight(TreeNode* node) const
{
    if (!node)
    {
        return 0;
    }
    int lh = calcHeight(node->left);
    int rh = calcHeight(node->right);
    
    return 1 + (lh > rh ? lh : rh);
}

int BST::height() const
{
    return calcHeight(m_root);
}

void BST::serializeHelper(TreeNode* node, string& out) const
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

string BST::serialize() const
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

static TreeNode* deserializeBSTHelper(const string& data, size_t& pos, TreeNode* parent, int& nextId)
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
    node->left = deserializeBSTHelper(data, pos, node, nextId);
    node->right = deserializeBSTHelper(data, pos, node, nextId);
    return node;
}

void BST::deserialize(const string& data)
{
    clear();
    size_t pos = 0;
    m_root = deserializeBSTHelper(data, pos, nullptr, m_nextId);
}
