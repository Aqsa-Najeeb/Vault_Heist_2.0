#include "BinaryTree.h"
#include "User.h"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>

using namespace std;
using namespace sf;
using json = nlohmann::json;

static float clampVal(float v, float low, float high)
{
    if (v < low)
    {
        return low;
    }

    if (v > high)
    {
        return high;
    }
    
    return v;
}

static int maxVal(int a, int b)
{
    return (a > b) ? a : b;
}

static bool isInside(float x, float y, float left, float top, float width, float height)
{
    return (x >= left && x <= left + width && y >= top && y <= top + height);
}

static void drawStar(RenderWindow& window, float cx, float cy, float radius, bool filled)
{
    ConvexShape star;
    star.setPointCount(10);

    for (int i = 0; i < 10; ++i)
    {
        float r = (i % 2 == 0) ? radius : radius * 0.45f;
        float angle = -1.5707963f + static_cast<float>(i) * 0.6283185f;
        star.setPoint(i, { cx + r * cos(angle), cy + r * sin(angle) });
    }

    if (filled)
    {
        star.setFillColor(Color(255, 215, 0));
        star.setOutlineThickness(1.5f);
        star.setOutlineColor(Color(200, 160, 0));
    }
    else
    {
        star.setFillColor(Color::Transparent);
        star.setOutlineThickness(1.5f);
        star.setOutlineColor(Color(140, 150, 160, 200));
    }

    window.draw(star);
}

static void drawThickLine(RenderWindow& window, Vector2f p1, Vector2f p2, float thickness, Color color)
{
    Vector2f dir = p2 - p1;
    float length = sqrt(dir.x * dir.x + dir.y * dir.y);
    
    if (length == 0.f) 
        return;

    RectangleShape line({ length, thickness });
    line.setOrigin({ 0.f, thickness * 0.5f });
    line.setPosition(p1);
    float angle = atan2(dir.y, dir.x) * 180.f / 3.14159265f;
    line.setRotation(degrees(angle));
    line.setFillColor(color);
    window.draw(line);
}

class EventLogger
{
public:
    string messages[32];
    int count = 0;

    void record(const string& msg)
    {
        if (count < 32)
        {
            messages[count++] = msg;
        }
        else
        {
            for (int i = 0; i < 31; ++i)
                messages[i] = messages[i + 1];
        
            messages[31] = msg;
        }
    }

    void undo()
    {
        if (count > 0)
            count--;
    }

    void clear()
    {
        count = 0;
    }
};

struct LevelScore
{
    int optimalSteps;
    int alarmCapacity;
    int alarmSteps;
    int penalties;
    bool hintUsed;
    int score;

    void reset(int optimal)
    {
        optimalSteps = optimal > 0 ? optimal : 1;
        alarmCapacity = static_cast<int>(ceil(1.5f * static_cast<float>(optimalSteps))) + 2;
        alarmSteps = 0;
        penalties = 0;
        hintUsed = false;
        score = 1000;
    }

    void addStep()
    {
        alarmSteps++;
        update();
    }

    void addPenalty(int cost = 1)
    {
        penalties++;
        alarmSteps += cost;
        update();
    }

    void undoStep(int cost = 1)
    {
        if (alarmSteps >= cost)
            alarmSteps -= cost;
        
        update();
    }

    void useHint()
    {
        hintUsed = true;
    }

    bool isAlarmFull() const
    {
        return (alarmSteps >= alarmCapacity);
    }

    int calculateStars() const
    {
        if (isAlarmFull()) 
            return 0;

        int stars = 1;
        float optF = static_cast<float>(optimalSteps);

        if (alarmSteps <= optimalSteps && penalties == 0)
            stars = 3;
        else if (static_cast<float>(alarmSteps) <= 1.25f * optF && penalties <= 1)
            stars = 2;
        else
            stars = 1;

        if (hintUsed && stars > 2)
            stars = 2;

        return stars;
    }

private:
    void update()
    {
        int calc = 1000 - 10 * alarmSteps - penalties * 20;
        score = maxVal(0, calc);
    }
};

struct LevelButton
{
    string id;
    string label;
    float x = 0.f;
    float y = 0.f;
    float w = 0.f;
    float h = 0.f;
    bool hasInput = false;
    float ix = 0.f;
    float iy = 0.f;
    float iw = 0.f;
    float ih = 0.f;
};

struct ArrayCell
{
    int index = 0;
    float x = 0.f;
    float y = 0.f;
    float w = 0.f;
    float h = 0.f;
};

struct NodePosition
{
    int id = 0;
    int value = 0;
    float x = 0.f;
    float y = 0.f;
    float radius = 30.f;
    int parentId = -1;
    int leftId = -1;
    int rightId = -1;

    bool isVisited = false;
    int visitIdx = 0;
    bool isHighlighted = false;
    bool isSelected = false;
    bool isViolated = false;
    bool isGreyed = false;
    bool isLCA = false;
    bool isPath = false;
    string badgeText = "";
    Color badgeColor = Color(255, 215, 0);
};

static void computeInorder(TreeNode* node, int depth, int& order, TreeNode* orderNodes[], int orderDepths[], int& maxDepth)
{
    if (!node)
    {
        return;
    }

    if (depth > maxDepth)
    {
        maxDepth = depth;
    }
    
    computeInorder(node->left, depth + 1, order, orderNodes, orderDepths, maxDepth);
    orderDepths[order] = depth;
    orderNodes[order++] = node;
    computeInorder(node->right, depth + 1, order, orderNodes, orderDepths, maxDepth);
}

static void adjustParentCenters(TreeNode* node, NodePosition positions[], int posCount)
{
    if (!node)
    {
        return;
    }

    adjustParentCenters(node->left, positions, posCount);
    adjustParentCenters(node->right, positions, posCount);

    if (node->left && node->right)
    {
        float lx = 0.f, rx = 0.f;
        int found = 0;
    
        for (int i = 0; i < posCount; ++i)
        {
            if (positions[i].id == node->left->id) 
            { 
                lx = positions[i].x; 
                found++; 
            }
            
            if (positions[i].id == node->right->id) 
            { 
                rx = positions[i].x; 
                found++; 
            }
        }

        if (found == 2)
        {
            for (int i = 0; i < posCount; ++i)
            {
                if (positions[i].id == node->id)
                {
                    positions[i].x = (lx + rx) * 0.5f;
                    break;
                }
            }
        }
    }
}

static int layoutBinaryTree(TreeNode* root, float canvasX, float canvasY, float canvasW, float canvasH, NodePosition positions[], int maxPositions)
{
    if (!root)
    {
        return 0;
    }

    TreeNode* orderNodes[32];
    int orderDepths[32];
    int totalNodes = 0;
    int maxDepth = 0;

    computeInorder(root, 0, totalNodes, orderNodes, orderDepths, maxDepth);

    if (totalNodes > maxPositions) 
        totalNodes = maxPositions;

    const float padX = 70.f;
    const float padTop = 50.f;
    const float padBottom = 55.f;

    float usableW = canvasW - 2.f * padX;
    float usableH = canvasH - padTop - padBottom;

    float xStep = (totalNodes > 1) ? (usableW / static_cast<float>(totalNodes - 1)) : 0.f;
    float yStep = (maxDepth > 0) ? (usableH / static_cast<float>(maxDepth)) : 0.f;

    float radius = 32.f;
    
    if (totalNodes > 8) 
        radius = 28.f;
    
    if (totalNodes > 12) 
        radius = 24.f;

    for (int i = 0; i < totalNodes; ++i)
    {
        TreeNode* n = orderNodes[i];
        float nx = (totalNodes > 1) ? (canvasX + padX + static_cast<float>(i) * xStep) : (canvasX + canvasW * 0.5f);
        float ny = canvasY + padTop + static_cast<float>(orderDepths[i]) * yStep;

        positions[i].id = n->id;
        positions[i].value = n->value;
        positions[i].x = nx;
        positions[i].y = ny;
        positions[i].radius = radius;
        positions[i].parentId = n->parent ? n->parent->id : -1;
        positions[i].leftId = n->left ? n->left->id : -1;
        positions[i].rightId = n->right ? n->right->id : -1;
    }

    adjustParentCenters(root, positions, totalNodes);
    return totalNodes;
}

static int layoutHeapTree(const int arrayData[], int heapCount, float canvasX, float canvasY, float canvasW, float canvasH, NodePosition positions[], int maxPositions)
{
    if (heapCount <= 0)
    {
        return 0;
    }

    int count = heapCount;
    
    if (count > maxPositions)
    {
        count = maxPositions;
    }

    int maxLevel = 0;
    while ((1 << (maxLevel + 1)) - 1 < count)
    {
        maxLevel++;
    }

    const float padTop = 45.f;
    const float padBottom = 45.f;
    float usableH = canvasH - padTop - padBottom;
    float yStep = (maxLevel > 0) ? (usableH / static_cast<float>(maxLevel)) : 0.f;
    float radius = (count > 7) ? 26.f : 30.f;

    for (int i = 0; i < count; ++i)
    {
        int level = 0;
        int temp = i + 1;
     
        while (temp > 1) 
        { 
            temp >>= 1; 
            level++; 
        }

        int levelStart = (1 << level) - 1;
        int levelCount = 1 << level;
        int indexInLevel = i - levelStart;

        float xStep = canvasW / static_cast<float>(levelCount + 1);
        float nx = canvasX + static_cast<float>(indexInLevel + 1) * xStep;
        float ny = canvasY + padTop + static_cast<float>(level) * yStep;

        positions[i].id = i;
        positions[i].value = arrayData[i];
        positions[i].x = nx;
        positions[i].y = ny;
        positions[i].radius = radius;
        positions[i].parentId = (i > 0) ? ((i - 1) / 2) : -1;
        positions[i].leftId = (2 * i + 1 < count) ? (2 * i + 1) : -1;
        positions[i].rightId = (2 * i + 2 < count) ? (2 * i + 2) : -1;
    }

    return count;
}

static string showResultScreen(RenderWindow& window, bool isPass, int stars, int steps, int optimalSteps, int penalties, const Font& font)
{
    string bgPath = (isPass)? "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\result_pass.png" : "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\result_fail.png";
    Texture bgTex;
   
    if (!bgTex.loadFromFile(bgPath))
    {
        return "Back";
    }

    Sprite bgSprite(bgTex);
    const float starCenters[3][2] = 
    {
        { 840.f, 480.f },
        { 960.f, 480.f },
        { 1080.f, 480.f }
    };

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
            {
                window.close();
                return "Exit";
            }

            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape || kp->code == Keyboard::Key::Enter || kp->code == Keyboard::Key::Space)
                {
                    return isPass ? "Next" : "Retry";
                }
            }

            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    const Vector2u winSize = window.getSize();
                    const float sx = static_cast<float>(winSize.x) / 1920.f;
                    const float sy = static_cast<float>(winSize.y) / 1080.f;
                    const Vector2i mpos = Mouse::getPosition(window);
                    const float mx = static_cast<float>(mpos.x) / sx;
                    const float my = static_cast<float>(mpos.y) / sy;

                    if (isInside(mx, my, 60.f, 72.f, 95.f, 80.f))
                    {
                        return "Back";
                    }

                    if (isInside(mx, my, 518.f, 862.f, 884.f, 125.f))
                    {
                        return isPass ? "Next" : "Retry";
                    }
                }
            }
        }

        const Vector2u winSize = window.getSize();
    
        if (winSize.x == 0 || winSize.y == 0) 
            continue;
        
        const float sx = static_cast<float>(winSize.x) / 1920.f;
        const float sy = static_cast<float>(winSize.y) / 1080.f;

        bgSprite.setScale({ sx, sy });

        window.clear();
        window.draw(bgSprite);

        if (isPass)
        {
            for (int s = 0; s < 3; ++s)
            {
                drawStar(window, starCenters[s][0] * sx, starCenters[s][1] * sy, 40.f * sx, stars > s);
            }
        }

        Text stepsText(font, to_string(steps) + " / " + to_string(optimalSteps), static_cast<unsigned int>(42 * sy));
        stepsText.setFillColor(Color::White);
        stepsText.setPosition({ 1020.f * sx, 618.f * sy });
        window.draw(stepsText);

        Text penText(font, to_string(penalties), static_cast<unsigned int>(42 * sy));
        penText.setFillColor(penalties > 0 ? Color(255, 100, 100) : Color::White);
        penText.setPosition({ 1020.f * sx, 714.f * sy });
        window.draw(penText);

        window.display();
    }

    return "Back";
}

struct LevelContext
{
    int& visitedCount;
    int& historyCount;
    bool& isReplaying;
    int& hintNode;
    bool& isLevelDone;
    int& completedSteps;
    int& selectedHeapIdx;
    EventLogger& eventLog;
    const string& normId;
    BinaryTree& tree;
    int& nodePositionCount;
    float canvasX;
    float canvasY;
    float canvasW;
    float canvasH;
    NodePosition* nodePositions;
    int& targetSequenceSize;
    int* targetSequenceData;
    int& optimalSteps;
    LevelScore& scoreSystem;
    string& activeButtonId;
    int& reconStep;
    bool& diameterCalculated;
    bool* heightCalculated;
    bool& lcaIdentified;
    int& pathIndex;
    bool* mirroredNode;
    int& bstSearchIdx;
    int& bstInsertIdx;
    bool& bstCaseIdentified;
    bool& bstSuccessorSwapped;
    bool& bstDeleted;
    int& bstRangeIdx;
    const int* bstBossChain;
    int& bstBossIdx;
    bool* avlFoundUnbalanced;
    int& avlRotationStep;
    int& avlDoubleStep;
    int& avlPredictionIdx;
    int& kthIdx;
    bool isAct4;
    int& arraySize;
    int& activeHeapSize;
    int* heapArray;
    Progress& progress;
    RenderWindow& window;
    const Font& font;
};

static void setupLevel(LevelContext& ctx)
{
    int& visitedCount = ctx.visitedCount;
    int& historyCount = ctx.historyCount;
    bool& isReplaying = ctx.isReplaying;
    int& hintNode = ctx.hintNode;
    bool& isLevelDone = ctx.isLevelDone;
    int& completedSteps = ctx.completedSteps;
    int& selectedHeapIdx = ctx.selectedHeapIdx;
    EventLogger& eventLog = ctx.eventLog;
    const string& normId = ctx.normId;
    BinaryTree& tree = ctx.tree;
    int& nodePositionCount = ctx.nodePositionCount;
    float canvasX = ctx.canvasX;
    float canvasY = ctx.canvasY;
    float canvasW = ctx.canvasW;
    float canvasH = ctx.canvasH;
    NodePosition* nodePositions = ctx.nodePositions;
    int& targetSequenceSize = ctx.targetSequenceSize;
    int* targetSequenceData = ctx.targetSequenceData;
    int& optimalSteps = ctx.optimalSteps;
    LevelScore& scoreSystem = ctx.scoreSystem;
    string& activeButtonId = ctx.activeButtonId;
    int& reconStep = ctx.reconStep;
    bool& diameterCalculated = ctx.diameterCalculated;
    bool* heightCalculated = ctx.heightCalculated;
    bool& lcaIdentified = ctx.lcaIdentified;
    int& pathIndex = ctx.pathIndex;
    bool* mirroredNode = ctx.mirroredNode;
    int& bstSearchIdx = ctx.bstSearchIdx;
    int& bstInsertIdx = ctx.bstInsertIdx;
    bool& bstCaseIdentified = ctx.bstCaseIdentified;
    bool& bstSuccessorSwapped = ctx.bstSuccessorSwapped;
    bool& bstDeleted = ctx.bstDeleted;
    int& bstRangeIdx = ctx.bstRangeIdx;
    const int* bstBossChain = ctx.bstBossChain;
    int& bstBossIdx = ctx.bstBossIdx;
    bool* avlFoundUnbalanced = ctx.avlFoundUnbalanced;
    int& avlRotationStep = ctx.avlRotationStep;
    int& avlDoubleStep = ctx.avlDoubleStep;
    int& avlPredictionIdx = ctx.avlPredictionIdx;
    int& kthIdx = ctx.kthIdx;
    bool isAct4 = ctx.isAct4;
    int& arraySize = ctx.arraySize;
    int& activeHeapSize = ctx.activeHeapSize;
    int* heapArray = ctx.heapArray;

    visitedCount = 0;
    historyCount = 0;
    isReplaying = false;
    hintNode = -1;
    isLevelDone = false;
    completedSteps = 0;
    selectedHeapIdx = -1;
    eventLog.clear();

    if (normId == "1_1")
    {
        tree = BinaryTree::generateSeeded(1101, 7);
        nodePositionCount = layoutBinaryTree(tree.getRoot(), canvasX, canvasY, canvasW, canvasH, nodePositions, 32);

        TraversalResult tr = tree.preorder();
        targetSequenceSize = tr.size;
        for (int i = 0; i < tr.size; ++i) targetSequenceData[i] = tr.get(i);

        optimalSteps = targetSequenceSize;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "preorder_traversal";
        eventLog.record("[PATROL] Preorder route active");
        eventLog.record("[TARGET] Predict " + to_string(targetSequenceSize) + " rooms");
    }
    else if (normId == "1_2")
    {
        tree = BinaryTree::generateSeeded(1101, 7);
        nodePositionCount = layoutBinaryTree(tree.getRoot(), canvasX, canvasY, canvasW, canvasH, nodePositions, 32);

        optimalSteps = 7;
        reconStep = 0;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "build_from_preorder_plus_inorder";

        eventLog.record("[BLUEPRINT] Pre: 4, 2, 1, 3, 7, 6, 9");
        eventLog.record("[BLUEPRINT] In: 1, 2, 3, 4, 6, 7, 9");
        eventLog.record("[GOAL] Place 7 rooms by traversal");
    }
    else if (normId == "1_3")
    {
        tree = BinaryTree::generateSeeded(1101, 7);
        nodePositionCount = layoutBinaryTree(tree.getRoot(), canvasX, canvasY, canvasW, canvasH, nodePositions, 32);

        optimalSteps = 8; 
        diameterCalculated = false;
        
        for (int i = 0; i < 32; ++i) 
            heightCalculated[i] = false;
        
        scoreSystem.reset(optimalSteps);
        activeButtonId = "calculate_height";

        eventLog.record("[ALARM] Spreads bottom-up from leaves");
        eventLog.record("[TASK] Compute heights postorder, then diameter");
    }
    else if (normId == "1_4")
    {
        tree = BinaryTree::generateSeeded(1101, 7);
        nodePositionCount = layoutBinaryTree(tree.getRoot(), canvasX, canvasY, canvasW, canvasH, nodePositions, 32);

        optimalSteps = 6; 
        lcaIdentified = false;
        pathIndex = 0;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "find_lca";

        eventLog.record("[ROUTES] Escape A: 1 | Escape B: 6");
        eventLog.record("[TASK] Find LCA meeting room, then trace path");
    }
    else if (normId == "1_5")
    {
        tree = BinaryTree::generateSeeded(1101, 7);
        nodePositionCount = layoutBinaryTree(tree.getRoot(), canvasX, canvasY, canvasW, canvasH, nodePositions, 32);

        optimalSteps = 3; 
        
        for (int i = 0; i < 3; ++i) 
            mirroredNode[i] = false;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "swap_left_and_right_children";

        eventLog.record("[MIRROR] Tree inversion in progress");
        eventLog.record("[TASK] Swap children of 3 internal nodes");
    }
    else if (normId == "2_1")
    {
        tree = BinaryTree::generateSeeded(1101, 7);
        nodePositions[0] = { 1, 50, canvasX + canvasW * 0.5f, canvasY + 60.f, 32.f, -1, 2, 3 };
        nodePositions[1] = { 2, 30, canvasX + canvasW * 0.25f, canvasY + 190.f, 32.f, 1, 4, 5 };
        nodePositions[2] = { 3, 70, canvasX + canvasW * 0.75f, canvasY + 190.f, 32.f, 1, 6, 7 };
        nodePositions[3] = { 4, 20, canvasX + canvasW * 0.12f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositions[4] = { 5, 40, canvasX + canvasW * 0.38f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositions[5] = { 6, 60, canvasX + canvasW * 0.62f, canvasY + 340.f, 30.f, 3, -1, -1 };
        nodePositions[6] = { 7, 80, canvasX + canvasW * 0.88f, canvasY + 340.f, 30.f, 3, -1, -1 };
        nodePositionCount = 7;

        bstSearchIdx = 0;
        optimalSteps = 3; // 50 -> 70 -> 60
        scoreSystem.reset(optimalSteps);
        activeButtonId = "search";

        eventLog.record("[HEIST] Target key: 60");
        eventLog.record("[COMPARE] Navigate left (<) or right (>)");
    }
    else if (normId == "2_2")
    {
        nodePositions[0] = { 1, 50, canvasX + canvasW * 0.5f, canvasY + 60.f, 32.f, -1, 2, 3 };
        nodePositions[1] = { 2, 30, canvasX + canvasW * 0.25f, canvasY + 190.f, 32.f, 1, 4, -1 };
        nodePositions[2] = { 3, 70, canvasX + canvasW * 0.75f, canvasY + 190.f, 32.f, 1, 5, 6 };
        nodePositions[3] = { 4, 20, canvasX + canvasW * 0.12f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositions[4] = { 5, 60, canvasX + canvasW * 0.62f, canvasY + 340.f, 30.f, 3, -1, -1 };
        nodePositions[5] = { 6, 80, canvasX + canvasW * 0.88f, canvasY + 340.f, 30.f, 3, -1, -1 };
        nodePositionCount = 6;

        bstInsertIdx = 0;
        optimalSteps = 3; // 50 -> 30 -> place 35
        scoreSystem.reset(optimalSteps);
        activeButtonId = "insert";

        eventLog.record("[LOOT] Stash loot item 35 into BST");
        eventLog.record("[NAV] Trace path to correct leaf slot");
    }
    else if (normId == "2_3")
    {
        nodePositions[0] = { 1, 50, canvasX + canvasW * 0.5f, canvasY + 60.f, 32.f, -1, 2, 3 };
        nodePositions[1] = { 2, 30, canvasX + canvasW * 0.25f, canvasY + 190.f, 32.f, 1, 4, 5 };
        nodePositions[2] = { 3, 70, canvasX + canvasW * 0.75f, canvasY + 190.f, 32.f, 1, -1, 6 };
        nodePositions[3] = { 4, 20, canvasX + canvasW * 0.12f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositions[4] = { 5, 40, canvasX + canvasW * 0.38f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositions[5] = { 6, 80, canvasX + canvasW * 0.88f, canvasY + 340.f, 30.f, 3, -1, -1 };
        nodePositionCount = 6;

        bstCaseIdentified = false;
        bstSuccessorSwapped = false;
        bstDeleted = false;
        optimalSteps = 4;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "delete";

        eventLog.record("[COLLAPSE] Delete target room 30");
        eventLog.record("[CASE] Identify deletion case");
    }
    else if (normId == "2_4")
    {
        nodePositions[0] = { 1, 50, canvasX + canvasW * 0.5f, canvasY + 60.f, 32.f, -1, 2, 3 };
        nodePositions[1] = { 2, 30, canvasX + canvasW * 0.25f, canvasY + 190.f, 32.f, 1, 4, 5 };
        nodePositions[2] = { 3, 70, canvasX + canvasW * 0.75f, canvasY + 190.f, 32.f, 1, 6, 7 };
        nodePositions[3] = { 4, 20, canvasX + canvasW * 0.12f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositions[4] = { 5, 40, canvasX + canvasW * 0.38f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositions[5] = { 6, 60, canvasX + canvasW * 0.62f, canvasY + 340.f, 30.f, 3, -1, -1 };
        nodePositions[6] = { 7, 80, canvasX + canvasW * 0.88f, canvasY + 340.f, 30.f, 3, -1, -1 };
        nodePositionCount = 7;

        nodePositions[3].isGreyed = true;
        nodePositions[6].isGreyed = true;

        bstRangeIdx = 0;
        optimalSteps = 4; // 40, 50, 60, 70
        scoreSystem.reset(optimalSteps);
        activeButtonId = "execute_range_query";

        eventLog.record("[RANGE] Query interval [35, 75]");
        eventLog.record("[PRUNED] Subtrees <35 and >75 greyed out");
        eventLog.record("[TASK] Collect 4 rooms in inorder");
    }
    else if (normId == "2_5")
    {
        for (int i = 0; i < 6; ++i)
        {
            nodePositions[i] = { i + 1, bstBossChain[i], canvasX + 150.f + static_cast<float>(i) * 120.f, canvasY + 60.f + static_cast<float>(i) * 75.f, 28.f, (i > 0) ? i : -1, -1, (i < 5) ? (i + 2) : -1 };
        }

        nodePositionCount = 6;
        bstBossIdx = 0;
        optimalSteps = 6;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "search";

        eventLog.record("[BOSS] Lazy Architect's Corridor");
        eventLog.record("[WARNING] Degenerate BST: Height = 6");
        eventLog.record("[TASK] Reach treasure 60 before alarm triggers");
    }
    else if (normId == "3_1")
    {
        nodePositions[0] = { 1, 50, canvasX + canvasW * 0.5f, canvasY + 60.f, 32.f, -1, 2, -1 };
        nodePositions[1] = { 2, 30, canvasX + canvasW * 0.35f, canvasY + 190.f, 32.f, 1, 3, -1 };
        nodePositions[2] = { 3, 20, canvasX + canvasW * 0.22f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositionCount = 3;

        nodePositions[0].badgeText = "BF:+2";
        nodePositions[0].badgeColor = Color(255, 70, 70);
        nodePositions[0].isViolated = true;

        nodePositions[1].badgeText = "BF:+1";
        nodePositions[1].badgeColor = Color(0, 200, 100);

        nodePositions[2].badgeText = "BF:0";
        nodePositions[2].badgeColor = Color(0, 200, 100);

        avlFoundUnbalanced[0] = false;
        optimalSteps = 1;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "search_unbalanced_node";

        eventLog.record("[AVL] Badges show balance factor BF");
        eventLog.record("[VIOLATION] Red badge indicates |BF| > 1");
        eventLog.record("[TASK] Click the unbalanced room (BF:+2)");
    }
    else if (normId == "3_2")
    {
        //LL
        nodePositions[0] = { 1, 50, canvasX + canvasW * 0.5f, canvasY + 60.f, 32.f, -1, 2, -1 };
        nodePositions[1] = { 2, 30, canvasX + canvasW * 0.35f, canvasY + 190.f, 32.f, 1, 3, -1 };
        nodePositions[2] = { 3, 20, canvasX + canvasW * 0.22f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositionCount = 3;

        avlRotationStep = 0;
        optimalSteps = 2;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "select_pivot";

        eventLog.record("[REPAIR] Single rotation: LL Case");
        eventLog.record("[TASK] Select pivot 50 and Right Rotate");
    }
    else if (normId == "3_3")
    {
        //LR
        nodePositions[0] = { 1, 50, canvasX + canvasW * 0.5f, canvasY + 60.f, 32.f, -1, 2, -1 };
        nodePositions[1] = { 2, 30, canvasX + canvasW * 0.32f, canvasY + 190.f, 32.f, 1, -1, 3 };
        nodePositions[2] = { 3, 40, canvasX + canvasW * 0.44f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositionCount = 3;

        avlDoubleStep = 0;
        optimalSteps = 3;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "identify_rotation_case";

        eventLog.record("[REPAIR] Double rotation required");
        eventLog.record("[TASK] Identify LR: Left Rotate 30, then Right Rotate 50");
    }
    else if (normId == "3_4")
    {
        tree = BinaryTree::generateSeeded(1101, 3);
        nodePositionCount = layoutBinaryTree(tree.getRoot(), canvasX, canvasY, canvasW, canvasH, nodePositions, 32);

        avlPredictionIdx = 0;
        optimalSteps = 4;
        scoreSystem.reset(optimalSteps);
        activeButtonId = "predict_rotation";

        eventLog.record("[PREDICT] Predict rotation before insert");
        eventLog.record("[QUEUE] Inserting keys: 25, 15, 35, 10");
    }
    else if (normId == "3_5")
    {
        nodePositions[0] = { 1, 40, canvasX + canvasW * 0.5f, canvasY + 60.f, 32.f, -1, 2, 3 };
        nodePositions[1] = { 2, 20, canvasX + canvasW * 0.28f, canvasY + 190.f, 32.f, 1, 4, 5 };
        nodePositions[2] = { 3, 60, canvasX + canvasW * 0.72f, canvasY + 190.f, 32.f, 1, 6, 7 };
        nodePositions[3] = { 4, 10, canvasX + canvasW * 0.16f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositions[4] = { 5, 30, canvasX + canvasW * 0.40f, canvasY + 340.f, 30.f, 2, -1, -1 };
        nodePositions[5] = { 6, 50, canvasX + canvasW * 0.60f, canvasY + 340.f, 30.f, 3, -1, -1 };
        nodePositions[6] = { 7, 70, canvasX + canvasW * 0.84f, canvasY + 340.f, 30.f, 3, -1, -1 };
        nodePositionCount = 7;

        nodePositions[0].badgeText = "S:7";
        nodePositions[1].badgeText = "S:3";
        nodePositions[2].badgeText = "S:3";
        nodePositions[3].badgeText = "S:1";
        nodePositions[4].badgeText = "S:1";
        nodePositions[5].badgeText = "S:1";
        nodePositions[6].badgeText = "S:1";

        kthIdx = 0;
        optimalSteps = 3; // 40 -> 20 -> 30
        scoreSystem.reset(optimalSteps);
        activeButtonId = "find_kth_smallest";

        eventLog.record("[BOSS] Order-Statistic Heist");
        eventLog.record("[MISSION] Steal 4th smallest treasure in O(log n)");
        eventLog.record("[LOGIC] Compare k against left->size + 1");
    }
    else if (isAct4)
    {
        if (normId == "4_1")
        {
            int initVals[7] = { 15, 30, 22, 60, 45, 80, 50 };
            arraySize = 7;
            activeHeapSize = 7;
        
            for (int i = 0; i < 7; ++i) 
                heapArray[i] = initVals[i];

            optimalSteps = 4;
            scoreSystem.reset(optimalSteps);
            activeButtonId = "swap_selected";

            eventLog.record("[ACT 4] The Heap Fortress");
            eventLog.record("[HEAP_BUILD] Bottom-up Heap Construction");
            eventLog.record("[TASK] Swap parent < child to enforce Max Heap");
        }
        else if (normId == "4_2")
        {
            int initVals[7] = { 18, 85, 70, 60, 55, 40, 35 };
            arraySize = 7;
            activeHeapSize = 7;
            for (int i = 0; i < 7; ++i) heapArray[i] = initVals[i];

            optimalSteps = 2;
            scoreSystem.reset(optimalSteps);
            activeButtonId = "sift_down";

            eventLog.record("[REPAIR] Sift-down disturbed root 18");
            eventLog.record("[TASK] Swap root with largest child until valid");
        }
        else if (normId == "4_3")
        {
            int initVals[7] = { 90, 75, 80, 60, 50, 45, 30 };
            arraySize = 7;
            activeHeapSize = 7;
           
            for (int i = 0; i < 7; ++i) 
                heapArray[i] = initVals[i];

            optimalSteps = 6; 
            scoreSystem.reset(optimalSteps);
            activeButtonId = "extract_maximum";

            eventLog.record("[EXTRACT] Extract 3 maximum treasures");
            eventLog.record("[STEP] Swap root with last element, shrink heap");
        }
        else if (normId == "4_4")
        {
            int initVals[8] = { 88, 88, 75, 60, 50, 75, 40, 30 };
            arraySize = 8;
            activeHeapSize = 8;
            
            for (int i = 0; i < 8; ++i) 
                heapArray[i] = initVals[i];

            optimalSteps = 6; 
            scoreSystem.reset(optimalSteps);
            activeButtonId = "extract_maximum";

            eventLog.record("[RACE] Duplicate-heavy Heap Sort");
            eventLog.record("[TASK] Execute extractions under alarm limit");
        }
        else if (normId == "4_5")
        {
            int initVals[7] = { 35, 70, 20, 85, 50, 95, 60 };
            arraySize = 7;
            activeHeapSize = 7;
            
            for (int i = 0; i < 7; ++i) 
                heapArray[i] = initVals[i];

            optimalSteps = 8; 
            scoreSystem.reset(optimalSteps);
            activeButtonId = "build_max_heap";

            eventLog.record("[BOSS] Final Vault Chamber");
            eventLog.record("[HEAP_SORT] Complete full Heap Sort");
            eventLog.record("[OBJECTIVE] Sort array in ascending order");
        }

        nodePositionCount = layoutHeapTree(heapArray, activeHeapSize, canvasX, canvasY, canvasW, canvasH, nodePositions, 32);
    }
}

static bool isMaxHeapValid(const int heapArray[], int activeHeapSize)
{
    for (int i = 0; i < activeHeapSize; ++i)
    {
        int l = 2 * i + 1;
        int r = 2 * i + 2;
     
        if (l < activeHeapSize && heapArray[i] < heapArray[l]) 
            return false;
        
        if (r < activeHeapSize && heapArray[i] < heapArray[r]) 
            return false;
    }
    return true;
}

static string triggerVictory(LevelContext& ctx)
{
    ctx.isLevelDone = true;
    ctx.eventLog.record("[VICTORY] Objective accomplished!");

    if (ctx.normId == "4_5")
    {
        ctx.eventLog.record("[HEAP_SORT_COMPLETE] Final Vault Unlocked!");
    }
    
    int stars = ctx.scoreSystem.calculateStars();
    recordResult(ctx.progress, ctx.normId, stars);
    string res = showResultScreen(ctx.window, true, stars, ctx.scoreSystem.alarmSteps, ctx.optimalSteps, ctx.scoreSystem.penalties, ctx.font);
    return res;
}

static void checkToolbarHover(RenderWindow& window, float mouseX, float mouseY, float scaleX, float scaleY, const float r[4])
{
    if (isInside(mouseX, mouseY, r[0], r[1], r[2], r[3]))
    {
        RectangleShape hBox;
        hBox.setPosition({ r[0] * scaleX, r[1] * scaleY });
        hBox.setSize({ r[2] * scaleX, r[3] * scaleY });
        hBox.setFillColor(Color(255, 255, 255, 25));
        hBox.setOutlineThickness(2.5f * scaleX);
        hBox.setOutlineColor(Color(0, 191, 99, 200));
        window.draw(hBox);
    }
}

string runLevel(RenderWindow& window, const string& levelId, Progress& progress);

string runLevel(RenderWindow& window, const string& levelId)
{
    Progress progress;
    loadProgress(progress);
    return runLevel(window, levelId, progress);
}

string runLevel(RenderWindow& window, const string& levelId, Progress& progress)
{
    string normId = levelId;
    
    for (char& c : normId) 
        if (c == '.') c = '_';
    
    string dotId = levelId;
    
    for (char& c : dotId) 
        if (c == '_') c = '.';

    bool isAct4 = (normId.rfind("4_", 0) == 0);

    float canvasX = 398.f, canvasY = 265.f, canvasW = 1100.f, canvasH = isAct4 ? 525.f : 648.f;
    float eventLogX = 1523.f, eventLogY = 257.f, eventLogW = 343.f, eventLogH = 648.f;
    float alarmSlotX = 1085.f, alarmSlotY = 140.f;
    float scoreSlotX = 1360.f, scoreSlotY = 150.f;

    float undoBtn[4] = { 200.f, 955.f, 140.f, 70.f };
    float resetBtn[4] = { 518.f, 955.f, 127.f, 70.f };
    float replayBtn[4] = { 808.f, 955.f, 154.f, 70.f };
    float hintBtn[4] = { 1112.f, 955.f, 106.f, 70.f };
    float speedSlider[4] = { 1540.f, 975.f, 218.f, 30.f };
    float backArrow[4] = { 50.f, 28.f, 58.f, 44.f };

    LevelButton levelButtons[16];
    int levelButtonCount = 0;
    ArrayCell arrayCells[16];
    int arrayCellCount = 0;

    int seed = 1101;
    int nodeCount = 7;
    string taskType = "traversal";
    string bgFileName = "level_" + normId + ".png";

    {
        ifstream layoutFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\data\\vault_layout.json");
        if (layoutFile.is_open())
        {
            try
            {
                json layoutJson;
                layoutFile >> layoutJson;
                if (layoutJson.contains("shared"))
                {
                    const auto& shared = layoutJson["shared"];
                    if (shared.contains("canvas"))
                    {
                        canvasX = shared["canvas"].value("x", canvasX);
                        canvasY = shared["canvas"].value("y", canvasY);
                        canvasW = shared["canvas"].value("w", canvasW);
                        canvasH = shared["canvas"].value("h", canvasH);
                    }
                    if (shared.contains("event_log"))
                    {
                        eventLogX = shared["event_log"].value("x", eventLogX);
                        eventLogY = shared["event_log"].value("y", eventLogY);
                        eventLogW = shared["event_log"].value("w", eventLogW);
                        eventLogH = shared["event_log"].value("h", eventLogH);
                    }
                    if (shared.contains("hud"))
                    {
                        if (shared["hud"].contains("alarm_slot"))
                        {
                            alarmSlotX = shared["hud"]["alarm_slot"].value("x", alarmSlotX);
                            alarmSlotY = shared["hud"]["alarm_slot"].value("y", alarmSlotY);
                        }
                        if (shared["hud"].contains("score_slot"))
                        {
                            scoreSlotX = shared["hud"]["score_slot"].value("x", scoreSlotX);
                            scoreSlotY = shared["hud"]["score_slot"].value("y", scoreSlotY);
                        }
                    }
                    if (shared.contains("toolbar"))
                    {
                        const auto& tb = shared["toolbar"];
                        if (tb.contains("undo"))
                        {
                            undoBtn[0] = tb["undo"].value("x", undoBtn[0]);
                            undoBtn[1] = tb["undo"].value("y", undoBtn[1]);
                            undoBtn[2] = tb["undo"].value("w", undoBtn[2]);
                            undoBtn[3] = tb["undo"].value("h", undoBtn[3]);
                        }
                        if (tb.contains("reset"))
                        {
                            resetBtn[0] = tb["reset"].value("x", resetBtn[0]);
                            resetBtn[1] = tb["reset"].value("y", resetBtn[1]);
                            resetBtn[2] = tb["reset"].value("w", resetBtn[2]);
                            resetBtn[3] = tb["reset"].value("h", resetBtn[3]);
                        }
                        if (tb.contains("replay"))
                        {
                            replayBtn[0] = tb["replay"].value("x", replayBtn[0]);
                            replayBtn[1] = tb["replay"].value("y", replayBtn[1]);
                            replayBtn[2] = tb["replay"].value("w", replayBtn[2]);
                            replayBtn[3] = tb["replay"].value("h", replayBtn[3]);
                        }
                        if (tb.contains("hint"))
                        {
                            hintBtn[0] = tb["hint"].value("x", hintBtn[0]);
                            hintBtn[1] = tb["hint"].value("y", hintBtn[1]);
                            hintBtn[2] = tb["hint"].value("w", hintBtn[2]);
                            hintBtn[3] = tb["hint"].value("h", hintBtn[3]);
                        }
                        if (tb.contains("speed_slider"))
                        {
                            speedSlider[0] = tb["speed_slider"].value("x", speedSlider[0]);
                            speedSlider[1] = tb["speed_slider"].value("y", speedSlider[1]);
                            speedSlider[2] = tb["speed_slider"].value("w", speedSlider[2]);
                            speedSlider[3] = tb["speed_slider"].value("h", speedSlider[3]);
                        }
                    }
                    if (shared.contains("back_arrow"))
                    {
                        backArrow[0] = shared["back_arrow"].value("x", backArrow[0]);
                        backArrow[1] = shared["back_arrow"].value("y", backArrow[1]);
                        backArrow[2] = shared["back_arrow"].value("w", backArrow[2]);
                        backArrow[3] = shared["back_arrow"].value("h", backArrow[3]);
                    }
                }
                if (layoutJson.contains("levels") && layoutJson["levels"].contains(normId))
                {
                    const auto& lvl = layoutJson["levels"][normId];
                    if (lvl.contains("image"))
                        bgFileName = lvl["image"].get<string>();

                    if (lvl.contains("tree_area"))
                    {
                        canvasX = lvl["tree_area"].value("x", canvasX);
                        canvasY = lvl["tree_area"].value("y", canvasY);
                        canvasW = lvl["tree_area"].value("w", canvasW);
                        canvasH = lvl["tree_area"].value("h", 525.f);
                    }

                    if (lvl.contains("buttons") && lvl["buttons"].is_array())
                    {
                        for (const auto& btn : lvl["buttons"])
                        {
                            LevelButton b;
                            b.id = btn.value("id", "");
                            b.label = btn.value("label", "");
                            b.x = btn.value("x", 54.f);
                            b.y = btn.value("y", 384.f);
                            b.w = btn.value("w", 251.f);
                            b.h = btn.value("h", 77.f);
                            if (btn.contains("input"))
                            {
                                b.hasInput = true;
                                b.ix = btn["input"].value("x", 0.f);
                                b.iy = btn["input"].value("y", 0.f);
                                b.iw = btn["input"].value("w", 0.f);
                                b.ih = btn["input"].value("h", 0.f);
                            }
                            if (levelButtonCount < 16)
                            {
                                levelButtons[levelButtonCount++] = b;
                            }
                        }
                    }

                    if (lvl.contains("array_strip") && lvl["array_strip"].contains("cells"))
                    {
                        for (const auto& c : lvl["array_strip"]["cells"])
                        {
                            ArrayCell ac;
                            ac.index = c.value("index", 0);
                            ac.x = c.value("x", 520.f);
                            ac.y = c.value("y", 820.f);
                            ac.w = c.value("w", 88.f);
                            ac.h = c.value("h", 64.f);
                            if (arrayCellCount < 16)
                            {
                                arrayCells[arrayCellCount++] = ac;
                            }
                        }
                    }
                }
            }
            catch (...) {}
        }
    }

    if (isAct4 && arrayCellCount == 0)
    {
        for (int i = 0; i < 10; ++i)
        {
            ArrayCell ac;
            ac.index = i;
            ac.x = 520.f + static_cast<float>(i) * 88.f;
            ac.y = 820.f;
            ac.w = (i == 0 || i == 9) ? 128.f : 88.f;
            ac.h = 64.f;
    
            if (arrayCellCount < 16)
            {
                arrayCells[arrayCellCount++] = ac;
            }
        }
    }

    {
        ifstream levelsFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\data\\levels.json");
        if (levelsFile.is_open())
        {
            try
            {
                json lvlData;
                levelsFile >> lvlData;
                if (lvlData.contains("levels") && lvlData["levels"].is_array())
                {
                    for (const auto& item : lvlData["levels"])
                    {
                        string id = item.value("id", "");
                        if (id == dotId || id == normId)
                        {
                            seed = item.value("seed", seed);
                            taskType = item.value("task", taskType);
                            if (item.contains("generator") && item["generator"].contains("nodes"))
                            {
                                const auto& nodes = item["generator"]["nodes"];
                                if (nodes.is_array() && !nodes.empty())
                                {
                                    nodeCount = nodes[0].get<int>();
                                }
                            }
                            break;
                        }
                    }
                }
            }
            catch (...) {}
        }
    }

    string imagePath = "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\" + bgFileName;
    Texture backgroundTexture;
    
    if (!backgroundTexture.loadFromFile(imagePath))
    {
        return "Can't_load_image";
    }
    Sprite background(backgroundTexture);

    Font font;
    
    if (!font.openFromFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\fonts\\perandory-semi-condensed.otf"))
    {
        (void)font.openFromFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\fonts\\CinzelDecorative-Regular.ttf");
    }

    BinaryTree tree;
    NodePosition nodePositions[32];
    int nodePositionCount = 0;

    int heapArray[10] = { 0 };
    int arraySize = 7;
    int activeHeapSize = 7;
    int selectedHeapIdx = -1;

    LevelScore scoreSystem;
    EventLogger eventLog;

    string activeButtonId = (levelButtonCount == 0) ? "" : levelButtons[0].id;
    int optimalSteps = 7;
    int completedSteps = 0;
    bool isLevelDone = false;

    int targetSequenceData[32];
    int targetSequenceSize = 0;
    int visitedNodes[32];
    int visitedCount = 0;
    int visitOrderHistory[32];
    int historyCount = 0;

    int hintNode = -1;
    Clock hintClock;
    float speedRatio = 0.35f;
    float speedMultiplier = 1.0f;
    bool isReplaying = false;
    int replayStep = 0;
    Clock replayClock;
    bool draggingSlider = false;

    int reconStep = 0;
    int preSeq[7] = { 4, 2, 1, 3, 7, 6, 9 };
    int inSeq[7] = { 1, 2, 3, 4, 6, 7, 9 };

    bool heightCalculated[32] = { false };
    bool diameterCalculated = false;
    int treeDiameterValue = 5;

    int escapeRoomA = 1;
    int escapeRoomB = 6;
    int meetingLCA = 4;
    bool lcaIdentified = false;
    int pathSequence[5] = { 1, 2, 4, 7, 6 };
    int pathIndex = 0;

    int mirrorNodes[3] = { 2, 7, 4 };
    bool mirroredNode[3] = { false, false, false };

    int bstSearchTarget = 60;
    int bstSearchSteps[3] = { 50, 70, 60 };
    int bstSearchIdx = 0;

    int bstInsertVal = 35;
    int bstInsertSteps[3] = { 50, 30, 35 };
    int bstInsertIdx = 0;

    int bstDeleteTarget = 30;
    int bstDeleteCase = 3; 
    bool bstCaseIdentified = false;
    bool bstSuccessorSwapped = false;
    bool bstDeleted = false;

    int bstRangeLo = 35, bstRangeHi = 75;
    int bstRangeNodes[4] = { 40, 50, 60, 70 };
    int bstRangeIdx = 0;

    int bstBossChain[6] = { 10, 20, 30, 40, 50, 60 };
    int bstBossIdx = 0;

    int avlUnbalancedNodes[2] = { 50, 30 };
    bool avlFoundUnbalanced[2] = { false, false };

    int avlRotationStep = 0;
    int avlDoubleStep = 0;
    int avlPredictionIdx = 0;
    int avlPredictKeys[4] = { 25, 15, 35, 10 };
    string predictedRotations[4] = { "None", "LL", "RR", "LR" };

    int kthTarget = 4;
    int kthPath[3] = { 40, 20, 30 };
    int kthIdx = 0;

    LevelContext ctx {
        visitedCount,
        historyCount,
        isReplaying,
        hintNode,
        isLevelDone,
        completedSteps,
        selectedHeapIdx,
        eventLog,
        normId,
        tree,
        nodePositionCount,
        canvasX,
        canvasY,
        canvasW,
        canvasH,
        nodePositions,
        targetSequenceSize,
        targetSequenceData,
        optimalSteps,
        scoreSystem,
        activeButtonId,
        reconStep,
        diameterCalculated,
        heightCalculated,
        lcaIdentified,
        pathIndex,
        mirroredNode,
        bstSearchIdx,
        bstInsertIdx,
        bstCaseIdentified,
        bstSuccessorSwapped,
        bstDeleted,
        bstRangeIdx,
        bstBossChain,
        bstBossIdx,
        avlFoundUnbalanced,
        avlRotationStep,
        avlDoubleStep,
        avlPredictionIdx,
        kthIdx,
        isAct4,
        arraySize,
        activeHeapSize,
        heapArray,
        progress,
        window,
        font
    };

    LevelContext ctx {
        visitedCount,
        historyCount,
        isReplaying,
        hintNode,
        isLevelDone,
        completedSteps,
        selectedHeapIdx,
        eventLog,
        normId,
        tree,
        nodePositionCount,
        canvasX,
        canvasY,
        canvasW,
        canvasH,
        nodePositions,
        targetSequenceSize,
        targetSequenceData,
        optimalSteps,
        scoreSystem,
        activeButtonId,
        reconStep,
        diameterCalculated,
        heightCalculated,
        lcaIdentified,
        pathIndex,
        mirroredNode,
        bstSearchIdx,
        bstInsertIdx,
        bstCaseIdentified,
        bstSuccessorSwapped,
        bstDeleted,
        bstRangeIdx,
        bstBossChain,
        bstBossIdx,
        avlFoundUnbalanced,
        avlRotationStep,
        avlDoubleStep,
        avlPredictionIdx,
        kthIdx,
        isAct4,
        arraySize,
        activeHeapSize,
        heapArray,
        progress,
        window,
        font
    };

    setupLevel(ctx);

    while (window.isOpen())
    {
        const Vector2u windowSize = window.getSize();

        if (windowSize.x == 0 || windowSize.y == 0) 
            continue;

        const float scaleX = static_cast<float>(windowSize.x) / 1920.f;
        const float scaleY = static_cast<float>(windowSize.y) / 1080.f;

        const Vector2i mousePixel = Mouse::getPosition(window);
        const float mouseX = static_cast<float>(mousePixel.x) / scaleX;
        const float mouseY = static_cast<float>(mousePixel.y) / scaleY;

        // Replay
        if (isReplaying)
        {
            float delay = 0.6f / speedMultiplier;
            if (replayClock.getElapsedTime().asSeconds() >= delay)
            {
                replayClock.restart();
                
                if (replayStep < historyCount)
                {
                    visitedNodes[visitedCount++] = visitOrderHistory[replayStep];
                    replayStep++;
                }
                else
                {
                    isReplaying = false;
                }
            }
        }

        //Hint
        if (hintNode != -1 && hintClock.getElapsedTime().asSeconds() > 2.5f)
        {
            hintNode = -1;
        }

        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
            {
                window.close();
                return "Exit";
            }

            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape)
                {
                    return "Back";
                }
                else if (kp->code == Keyboard::Key::Z)
                {
                    if (completedSteps > 0)
                    {
                        completedSteps--;
        
                        if (visitedCount > 0) 
                            visitedCount--;
                        
                        scoreSystem.undoStep(1);
                        eventLog.undo();
                        eventLog.record("[UNDO] Step reverted");
                    }
                }
                else if (kp->code == Keyboard::Key::R)
                {
                    setupLevel(ctx);
                }
                else if (kp->code == Keyboard::Key::H)
                {
                    scoreSystem.useHint();
                    eventLog.record("[HINT] Follow next highlighted action");
                }
            }

            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    //Back
                    if (isInside(mouseX, mouseY, backArrow[0], backArrow[1], backArrow[2], backArrow[3]))
                    {
                        return "Back";
                    }

                    //Toolba
                    if (isInside(mouseX, mouseY, undoBtn[0], undoBtn[1], undoBtn[2], undoBtn[3]))
                    {
                        if (completedSteps > 0)
                        {
                            completedSteps--;
                            
                            if (visitedCount > 0) 
                                visitedCount--;
                            
                            scoreSystem.undoStep(1);
                            eventLog.undo();
                            eventLog.record("[UNDO] Step reverted");
                        }
                    }
                    else if (isInside(mouseX, mouseY, resetBtn[0], resetBtn[1], resetBtn[2], resetBtn[3]))
                    {
                        setupLevel(ctx);
                    }
                    else if (isInside(mouseX, mouseY, replayBtn[0], replayBtn[1], replayBtn[2], replayBtn[3]))
                    {
                        if (historyCount > 0)
                        {
                            visitedCount = 0;
                            isReplaying = true;
                            replayStep = 0;
                            replayClock.restart();
                            eventLog.record("[REPLAY] Playing animation");
                        }
                    }
                    else if (isInside(mouseX, mouseY, hintBtn[0], hintBtn[1], hintBtn[2], hintBtn[3]))
                    {
                        scoreSystem.useHint();
                        eventLog.record("[HINT] Perform next required level step");
                    }
                    else if (isInside(mouseX, mouseY, speedSlider[0], speedSlider[1], speedSlider[2], speedSlider[3]))
                    {
                        draggingSlider = true;
                        speedRatio = clampVal((mouseX - speedSlider[0]) / speedSlider[2], 0.05f, 0.95f);
                        speedMultiplier = 0.5f + speedRatio * 2.5f;
                    }

                    //Level Buttons
                    for (int bIdx = 0; bIdx < levelButtonCount; ++bIdx)
                    {
                        const auto& btn = levelButtons[bIdx];
                        if (isInside(mouseX, mouseY, btn.x, btn.y, btn.w, btn.h))
                        {
                            activeButtonId = btn.id;

                            if (normId == "1_1")
                            {
                                TraversalResult tr;

                                if (btn.id == "preorder_traversal") 
                                { 
                                    tr = tree.preorder(); 
                                    eventLog.record("[MODE] Preorder"); 
                                }
                                else if (btn.id == "inorder_traversal") 
                                { 
                                    tr = tree.inorder(); 
                                    eventLog.record("[MODE] Inorder"); 
                                }
                                else if (btn.id == "postorder_traversal") 
                                { 
                                    tr = tree.postorder(); 
                                    eventLog.record("[MODE] Postorder"); 
                                }
                                else if (btn.id == "level_order") 
                                { 
                                    tr = tree.levelOrder(); 
                                    eventLog.record("[MODE] Level Order"); 
                                }

                                targetSequenceSize = tr.size;
                                
                                for (int i = 0; i < tr.size; ++i) 
                                    targetSequenceData[i] = tr.get(i);
                                
                                visitedCount = 0;
                                historyCount = 0;
                                scoreSystem.reset(targetSequenceSize);
                            }
                            else if (normId == "1_2")
                            {
                                if (btn.id == "submit_tree")
                                {
                                    if (reconStep >= 7)
                                    {
                                        string res = triggerVictory(ctx);

                                        if (res == "Exit") 
                                            return "Exit";
                                        
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Tree incomplete (" + to_string(reconStep) + "/7 rooms placed)");
                                    }
                                }
                                else if (btn.id == "clear_restart_construction")
                                {
                                    reconStep = 0;
                                    scoreSystem.reset(optimalSteps);
                                    eventLog.record("[RESTART] Blueprint reset");
                                }
                            }
                            else if (normId == "1_3")
                            {
                                if (btn.id == "calculate_diameter")
                                {
                                    if (!diameterCalculated)
                                    {
                                        diameterCalculated = true;
                                        completedSteps++;
                                        scoreSystem.addStep();
                                        eventLog.record("[DIAMETER] Tree Diameter = " + to_string(treeDiameterValue));
                                    }
                                }
                                else if (btn.id == "submit_answer")
                                {
                                    if (completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                            
                                        if (res == "Exit") 
                                            return "Exit";
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Complete all height & diameter steps first");
                                    }
                                }
                            }
                            else if (normId == "1_4")
                            {
                                if (btn.id == "submit_answer")
                                {
                                    if (completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                                        if (res == "Exit") return "Exit";
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] LCA and escape path must be fully traced");
                                    }
                                }
                            }
                            else if (normId == "1_5")
                            {
                                if (btn.id == "auto_mirror")
                                {
                                    for (int i = 0; i < 3; ++i)
                                    {
                                        if (!mirroredNode[i])
                                        {
                                            mirroredNode[i] = true;
                                            completedSteps++;
                                            scoreSystem.addStep();
                                            eventLog.record("[MIRROR] Swapped children of room " + to_string(mirrorNodes[i]));
                                        }
                                    }
                                }
                                else if (btn.id == "submit_tree")
                                {
                                    if (completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                            
                                        if (res == "Exit") 
                                            return "Exit";
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Mirror all 3 internal nodes first");
                                    }
                                }
                            }
                            else if (normId == "2_3")
                            {
                                if (btn.id == "identify_deletion_case")
                                {
                                    if (!bstCaseIdentified)
                                    {
                                        bstCaseIdentified = true;
                                        completedSteps++;
                                        scoreSystem.addStep();
                                        eventLog.record("[CASE] Two children: Inorder Successor is 40");
                                    }
                                }
                                else if (btn.id == "confirm_delete")
                                {
                                    if (completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                            
                                        if (res == "Exit") 
                                            return "Exit";
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Identify case and swap successor before confirming");
                                    }
                                }
                            }
                            else if (normId == "3_1")
                            {
                                if (btn.id == "submit_selection")
                                {
                                    if (completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                                        if (res == "Exit") return "Exit";
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Unbalanced room must be identified");
                                    }
                                }
                            }
                            else if (normId == "3_2")
                            {
                                if (btn.id == "right_rotation" && avlRotationStep == 1)
                                {
                                    avlRotationStep = 2;
                                    completedSteps++;
                                    scoreSystem.addStep();
                                    eventLog.record("[ROTATE] Right Rotation applied on pivot 50");
                                    
                                    nodePositions[0].value = 30; 
                                    nodePositions[0].badgeText = "BF:0";
                                    
                                    nodePositions[1].value = 20; 
                                    nodePositions[1].badgeText = "BF:0";
                                    
                                    nodePositions[2].value = 50; 
                                    nodePositions[2].badgeText = "BF:0";
                                }
                                else if (btn.id == "confirm_rotation")
                                {
                                    if (completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                                        
                                        if (res == "Exit") 
                                            return "Exit";
                                        
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Perform correct right rotation first");
                                    }
                                }
                            }
                            else if (normId == "3_3")
                            {
                                if (btn.id == "identify_rotation_case" && avlDoubleStep == 0)
                                {
                                    avlDoubleStep = 1;
                                    completedSteps++;
                                    scoreSystem.addStep();
                                    eventLog.record("[CASE] LR identified: Left rotate 30, Right rotate 50");
                                }
                                else if (btn.id == "rotate_left" && avlDoubleStep == 1)
                                {
                                    avlDoubleStep = 2;
                                    completedSteps++;
                                    scoreSystem.addStep();
                                    eventLog.record("[ROTATE] Left rotation on child 30 applied");
                                }
                                else if (btn.id == "rotate_right" && avlDoubleStep == 2)
                                {
                                    avlDoubleStep = 3;
                                    completedSteps++;
                                    scoreSystem.addStep();
                                    eventLog.record("[ROTATE] Right rotation on pivot 50 applied");
                                    
                                    nodePositions[0].value = 40;
                                    nodePositions[1].value = 30;
                                    nodePositions[2].value = 50;
                                }
                                else if (btn.id == "confirm_rotation")
                                {
                                    if (completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                                        
                                        if (res == "Exit") 
                                            return "Exit";
                                        
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Perform both rotations first");
                                    }
                                }
                            }
                            else if (normId == "3_4")
                            {
                                if (btn.id == "submit_prediction" || btn.id == "insert")
                                {
                                    if (avlPredictionIdx < 4)
                                    {
                                        int k = avlPredictKeys[avlPredictionIdx];
                                        string rot = predictedRotations[avlPredictionIdx];
                                        eventLog.record("[INSERT] Inserted " + to_string(k) + " -> Rotation: " + rot);
                                        avlPredictionIdx++;
                                        completedSteps++;
                                        scoreSystem.addStep();

                                        if (completedSteps >= optimalSteps)
                                        {
                                            string res = triggerVictory(ctx);
                            
                                            if (res == "Exit") 
                                                return "Exit";
                                            
                                            return "CAMPAIGN MAP";
                                        }
                                    }
                                }
                            }
                            else if (normId == "3_5")
                            {
                                if (btn.id == "submit_answer")
                                {
                                    if (completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                            
                                        if (res == "Exit") 
                                            return "Exit";
                                        
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Navigate to 4th smallest room first");
                                    }
                                }
                            }
                            else if (isAct4)
                            {
                                if (btn.id == "swap_selected")
                                {
                                    for (int p = 0; p < activeHeapSize; ++p)
                                    {
                                        int l = 2 * p + 1;
                                        int r = 2 * p + 2;
                                        int targetC = -1;
                            
                                        if (l < activeHeapSize && heapArray[l] > heapArray[p]) 
                                            targetC = l;
                                        
                                        if (r < activeHeapSize && heapArray[r] > heapArray[p] && (targetC == -1 || heapArray[r] > heapArray[l])) 
                                            targetC = r;

                                        if (targetC != -1)
                                        {
                                            int tmp = heapArray[p];
                                            heapArray[p] = heapArray[targetC];
                                            heapArray[targetC] = tmp;

                                            completedSteps++;
                                            scoreSystem.addStep();
                                            
                                            eventLog.record("[HEAP_SWAP] Swapped [" + to_string(p) + "] (" + to_string(heapArray[targetC]) + ") <-> [" + to_string(targetC) + "] (" + to_string(heapArray[p]) + ")");
                                            
                                            nodePositionCount = layoutHeapTree(heapArray, activeHeapSize, canvasX, canvasY, canvasW, canvasH, nodePositions, 32);
                                            break;
                                        }
                                    }
                                }
                                else if (btn.id == "confirm_heap" || btn.id == "confirm_parent_child_relationships")
                                {
                                    if (isMaxHeapValid(heapArray, activeHeapSize) && completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                                        if (res == "Exit") return "Exit";
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Heap property not fully satisfied (" + to_string(completedSteps) + "/" + to_string(optimalSteps) + " steps)");
                                    }
                                }
                                else if (btn.id == "sift_down")
                                {
                                    int p = 0;
                                    int l = 2 * p + 1;
                                    int r = 2 * p + 2;
                                    int largest = p;
                                    
                                    if (l < activeHeapSize && heapArray[l] > heapArray[largest]) 
                                        largest = l;
                                    
                                    if (r < activeHeapSize && heapArray[r] > heapArray[largest]) 
                                        largest = r;

                                    if (largest != p)
                                    {
                                        int tmp = heapArray[p];
                                        heapArray[p] = heapArray[largest];
                                        heapArray[largest] = tmp;

                                        completedSteps++;
                                        scoreSystem.addStep();
                                        eventLog.record("[HEAP_SWAP] Sifted [" + to_string(p) + "] <-> [" + to_string(largest) + "]");
                                        nodePositionCount = layoutHeapTree(heapArray, activeHeapSize, canvasX, canvasY, canvasW, canvasH, nodePositions, 32);
                                    }
                                }
                                else if (btn.id == "extract_maximum")
                                {
                                    if (activeHeapSize > 1)
                                    {
                                        int maxValExtracted = heapArray[0];
                                        int lastIdx = activeHeapSize - 1;
                                        heapArray[0] = heapArray[lastIdx];
                                        heapArray[lastIdx] = maxValExtracted;
                                        activeHeapSize--;

                                        completedSteps++;
                                        scoreSystem.addStep();
                                        eventLog.record("[EXTRACT_MAX] Root " + to_string(maxValExtracted) + " -> Sorted [" + to_string(lastIdx) + "]");
                                        nodePositionCount = layoutHeapTree(heapArray, activeHeapSize, canvasX, canvasY, canvasW, canvasH, nodePositions, 32);
                                    }
                                }
                                else if (btn.id == "heapify")
                                {
                                    int p = 0;
                                    
                                    while (true)
                                    {
                                        int l = 2 * p + 1;
                                        int r = 2 * p + 2;
                                        int largest = p;
                                        if (l < activeHeapSize && heapArray[l] > heapArray[largest]) largest = l;
                                        if (r < activeHeapSize && heapArray[r] > heapArray[largest]) largest = r;
                                        if (largest == p) break;

                                        int tmp = heapArray[p];
                                        heapArray[p] = heapArray[largest];
                                        heapArray[largest] = tmp;
                                        p = largest;
                                    }
                                    
                                    completedSteps++;
                                    scoreSystem.addStep();
                                    eventLog.record("[HEAPIFY_DONE] Active heap property restored");
                                    nodePositionCount = layoutHeapTree(heapArray, activeHeapSize, canvasX, canvasY, canvasW, canvasH, nodePositions, 32);
                                }
                                else if (btn.id == "confirm_extraction" || btn.id == "check_max_heap")
                                {
                                    if (completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                                        if (res == "Exit") return "Exit";
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Complete remaining extraction steps (" + to_string(completedSteps) + "/" + to_string(optimalSteps) + ")");
                                    }
                                }
                                else if (btn.id == "verify_sorted_array")
                                {
                                    bool sorted = true;
                                    for (int s = 0; s < arraySize - 1; ++s)
                                    {
                                        if (heapArray[s] > heapArray[s + 1]) { sorted = false; break; }
                                    }
                                    if (sorted && completedSteps >= optimalSteps)
                                    {
                                        string res = triggerVictory(ctx);
                                        if (res == "Exit") return "Exit";
                                        return "CAMPAIGN MAP";
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Array not yet sorted in ascending order");
                                    }
                                }
                            }
                            break;
                        }
                    }

                    if (isAct4)
                    {
                        for (int c = 0; c < arrayCellCount; ++c)
                        {
                            const auto& cell = arrayCells[c];
                            if (isInside(mouseX, mouseY, cell.x, cell.y, cell.w, cell.h) && static_cast<int>(c) < arraySize)
                            {
                                int clickedIdx = static_cast<int>(c);
                                if (selectedHeapIdx == -1)
                                {
                                    selectedHeapIdx = clickedIdx;
                                    eventLog.record("[SELECT] Array cell [" + to_string(clickedIdx) + "] (val: " + to_string(heapArray[clickedIdx]) + ")");
                                }
                                else
                                {
                                    int a = selectedHeapIdx;
                                    int b = clickedIdx;
                                    selectedHeapIdx = -1;

                                    if (a != b)
                                    {
                                        int tmp = heapArray[a];
                                        heapArray[a] = heapArray[b];
                                        heapArray[b] = tmp;

                                        completedSteps++;
                                        scoreSystem.addStep();
                                        eventLog.record("[HEAP_SWAP] Swapped [" + to_string(a) + "] <-> [" + to_string(b) + "]");
                                        nodePositionCount = layoutHeapTree(heapArray, activeHeapSize, canvasX, canvasY, canvasW, canvasH, nodePositions, 32);
                                    }
                                }
                                break;
                            }
                        }
                    }

                    if (!isReplaying)
                    {
                        for (int i = 0; i < nodePositionCount; ++i)
                        {
                            const auto& pos = nodePositions[i];
                            float dx = mouseX - pos.x;
                            float dy = mouseY - pos.y;
                            float dist = sqrt(dx * dx + dy * dy);

                            if (dist <= pos.radius)
                            {
                                int nodeVal = pos.value;

                                if (normId == "1_1")
                                {
                                    bool already = false;
                                    for (int v = 0; v < visitedCount; ++v)
                                    {
                                        if (visitedNodes[v] == nodeVal) { already = true; break; }
                                    }
                                    if (already) break;

                                    int expectedVal = targetSequenceData[visitedCount];
                                    if (nodeVal == expectedVal)
                                    {
                                        visitedNodes[visitedCount++] = nodeVal;
                                        visitOrderHistory[historyCount++] = nodeVal;
                                        scoreSystem.addStep();
                                        eventLog.record("[VISIT] Room " + to_string(nodeVal) +
                                                       " (" + to_string(visitedCount) + "/" + to_string(targetSequenceSize) + ")");

                                        if (visitedCount == targetSequenceSize)
                                        {
                                            string res = triggerVictory(ctx);
                                            
                                            if (res == "Exit") 
                                                return "Exit";
                                            
                                            return "CAMPAIGN MAP";
                                        }
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Wrong room " + to_string(nodeVal) + ", expected " + to_string(expectedVal) + " (+1 Alarm)");
                                    }
                                }
                                else if (normId == "1_2")
                                {
                                    if (reconStep < 7 && preSeq[reconStep] == nodeVal)
                                    {
                                        reconStep++;
                                        visitedNodes[visitedCount++] = nodeVal;
                                        scoreSystem.addStep();
                                        eventLog.record("[TOKEN] Reconstructed room " + to_string(nodeVal) + " (" + to_string(reconStep) + "/7)");
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Expected next room: " + to_string(preSeq[reconStep]));
                                    }
                                }
                                else if (normId == "1_3")
                                {
                                    if (!heightCalculated[i])
                                    {
                                        heightCalculated[i] = true;
                                        completedSteps++;
                                        scoreSystem.addStep();
                                        int hVal = 1;
                                        if (nodeVal == 2 || nodeVal == 7) hVal = 2;
                                        if (nodeVal == 4) hVal = 3;
                                        nodePositions[i].badgeText = "H:" + to_string(hVal);
                                        nodePositions[i].badgeColor = Color(0, 210, 110);
                                        eventLog.record("[HEIGHT] Room " + to_string(nodeVal) + " Height = " + to_string(hVal));
                                    }
                                }
                                else if (normId == "1_4")
                                {
                                    if (!lcaIdentified)
                                    {
                                        if (nodeVal == meetingLCA)
                                        {
                                            lcaIdentified = true;
                                            completedSteps++;
                                            scoreSystem.addStep();
                                            nodePositions[i].isLCA = true;
                                            nodePositions[i].badgeText = "LCA";
                                            nodePositions[i].badgeColor = Color(200, 100, 255);
                                            eventLog.record("[LCA] Meeting room 4 identified!");
                                        }
                                        else
                                        {
                                            scoreSystem.addPenalty(1);
                                            eventLog.record("[PENALTY] Room " + to_string(nodeVal) + " is not the meeting room");
                                        }
                                    }
                                    else if (pathIndex < 5)
                                    {
                                        if (nodeVal == pathSequence[pathIndex])
                                        {
                                            pathIndex++;
                                            completedSteps++;
                                            scoreSystem.addStep();
                                            nodePositions[i].isPath = true;
                                            eventLog.record("[PATH] Escape route node " + to_string(nodeVal) + " traced (" + to_string(pathIndex) + "/5)");
                                        }
                                        else
                                        {
                                            scoreSystem.addPenalty(1);
                                            eventLog.record("[PENALTY] Expected next path room: " + to_string(pathSequence[pathIndex]));
                                        }
                                    }
                                }
                                else if (normId == "1_5")
                                {
                                    for (int m = 0; m < 3; ++m)
                                    {
                                        if (mirrorNodes[m] == nodeVal && !mirroredNode[m])
                                        {
                                            mirroredNode[m] = true;
                                            completedSteps++;
                                            scoreSystem.addStep();
                                            nodePositions[i].isVisited = true;
                                            eventLog.record("[MIRROR] Children of room " + to_string(nodeVal) + " swapped");
                                            break;
                                        }
                                    }
                                }
                                else if (normId == "2_1")
                                {
                                    if (bstSearchIdx < 3 && nodeVal == bstSearchSteps[bstSearchIdx])
                                    {
                                        bstSearchIdx++;
                                        completedSteps++;
                                        scoreSystem.addStep();
                                        nodePositions[i].isVisited = true;
                                        
                                        if (bstSearchIdx < 3)
                                            eventLog.record("[COMPARE] Room " + to_string(nodeVal) + " -> Target is " + (bstSearchTarget > nodeVal ? "Larger (>)" : "Smaller (<)"));
                                        else
                                            eventLog.record("[FOUND] Target key 60 reached!");

                                        
                                        if (completedSteps >= optimalSteps)
                                        {
                                            string res = triggerVictory(ctx);
                                            if (res == "Exit") return "Exit";
                                            return "CAMPAIGN MAP";
                                        }
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Invalid BST navigation step");
                                    }
                                }
                                else if (normId == "2_2")
                                {
                                    if (bstInsertIdx < 3 && (nodeVal == bstInsertSteps[bstInsertIdx] || (bstInsertIdx == 2 && nodeVal == 30)))
                                    {
                                        bstInsertIdx++;
                                        completedSteps++;
                                        scoreSystem.addStep();
                                        nodePositions[i].isVisited = true;
                                        if (bstInsertIdx == 2)
                                        {
                                            nodePositions[nodePositionCount++] = { 7, 35, canvasX + canvasW * 0.38f, canvasY + 340.f, 30.f, 2, -1, -1 };
                                            nodePositions[nodePositionCount - 1].isVisited = true;
                                            eventLog.record("[INSERT] Stashed loot room 35 at correct leaf position");
                                        }
                                        else
                                        {
                                            eventLog.record("[NAV] Traversed to room " + to_string(nodeVal));
                                        }

                                        if (completedSteps >= optimalSteps)
                                        {
                                            string res = triggerVictory(ctx);
                                            if (res == "Exit") return "Exit";
                                            return "CAMPAIGN MAP";
                                        }
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Wrong insertion step");
                                    }
                                }
                                else if (normId == "2_3")
                                {
                                    if (nodeVal == 30)
                                    {
                                        nodePositions[i].isViolated = true;
                                        completedSteps = maxVal(completedSteps, 1);
                                        scoreSystem.addStep();
                                        eventLog.record("[TARGET] Room 30 selected for deletion");
                                    }
                                    else if (nodeVal == 40 && bstCaseIdentified)
                                    {
                                        bstSuccessorSwapped = true;
                                        completedSteps = maxVal(completedSteps, 3);
                                        scoreSystem.addStep();
                                        nodePositions[1].value = 40; //swap
                                        nodePositions[4].isGreyed = true; //unlink successor
                                        eventLog.record("[SWAP_VALUE] Replaced 30 with inorder successor 40");
                                    }
                                }
                                else if (normId == "2_4")
                                {
                                    if (bstRangeIdx < 4 && nodeVal == bstRangeNodes[bstRangeIdx])
                                    {
                                        bstRangeIdx++;
                                        completedSteps++;
                                        scoreSystem.addStep();
                                        nodePositions[i].isVisited = true;
                                        eventLog.record("[RANGE] Collected in-range treasure " + to_string(nodeVal) + " (" + to_string(bstRangeIdx) + "/4)");

                                        if (completedSteps >= optimalSteps)
                                        {
                                            string res = triggerVictory(ctx);
                                            if (res == "Exit") return "Exit";
                                            return "CAMPAIGN MAP";
                                        }
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Key not in range or not next in inorder");
                                    }
                                }
                                else if (normId == "2_5")
                                {
                                    if (bstBossIdx < 6 && nodeVal == bstBossChain[bstBossIdx])
                                    {
                                        bstBossIdx++;
                                        completedSteps++;
                                        scoreSystem.addStep();
                                        nodePositions[i].isVisited = true;
                                        eventLog.record("[CORRIDOR] Passed room " + to_string(nodeVal) + " (" + to_string(bstBossIdx) + "/6)");

                                        if (completedSteps >= optimalSteps)
                                        {
                                            eventLog.record("[CUTSCENE] Balancing is vital to escape O(N) chains!");
                                            string res = triggerVictory(ctx);
                                            if (res == "Exit") return "Exit";
                                            return "CAMPAIGN MAP";
                                        }
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Follow corridor in sequence");
                                    }
                                }
                                else if (normId == "3_1")
                                {
                                    if (nodeVal == 50)
                                    {
                                        avlFoundUnbalanced[0] = true;
                                        completedSteps = 1;
                                        scoreSystem.addStep();
                                        nodePositions[i].isSelected = true;
                                        eventLog.record("[REPORTED] Unbalanced room 50 (BF:+2) flagged!");
                                    }
                                }
                                else if (normId == "3_2")
                                {
                                    if (nodeVal == 50 && avlRotationStep == 0)
                                    {
                                        avlRotationStep = 1;
                                        completedSteps++;
                                        scoreSystem.addStep();
                                        nodePositions[i].isSelected = true;
                                        eventLog.record("[PIVOT] Pivot node 50 selected. Click 'Right Rotation'.");
                                    }
                                }
                                else if (normId == "3_5")
                                {
                                    if (kthIdx < 3 && nodeVal == kthPath[kthIdx])
                                    {
                                        kthIdx++;
                                        completedSteps++;
                                        scoreSystem.addStep();
                                        nodePositions[i].isVisited = true;
                                        eventLog.record("[ORDER_STAT] Stepped to room " + to_string(nodeVal));

                                        if (completedSteps >= optimalSteps)
                                        {
                                            nodePositions[i].isSelected = true;
                                            eventLog.record("[TARGET] Found 4th smallest treasure (30)! Click Submit.");
                                        }
                                    }
                                    else
                                    {
                                        scoreSystem.addPenalty(1);
                                        eventLog.record("[PENALTY] Follow order-statistic size path");
                                    }
                                }
                                else if (isAct4)
                                {
                                    int clickedHeapIdx = i;
                                    if (selectedHeapIdx == -1)
                                    {
                                        selectedHeapIdx = clickedHeapIdx;
                                        eventLog.record("[SELECT] Tree node [" + to_string(clickedHeapIdx) + "] (val: " + to_string(heapArray[clickedHeapIdx]) + ")");
                                    }
                                    else
                                    {
                                        int a = selectedHeapIdx;
                                        int b = clickedHeapIdx;
                                        selectedHeapIdx = -1;

                                        if (a != b)
                                        {
                                            int tmp = heapArray[a];
                                            heapArray[a] = heapArray[b];
                                            heapArray[b] = tmp;

                                            completedSteps++;
                                            scoreSystem.addStep();
                                            eventLog.record("[HEAP_SWAP] Swapped [" + to_string(a) + "] <-> [" + to_string(b) + "]");
                                            nodePositionCount = layoutHeapTree(heapArray, activeHeapSize, canvasX, canvasY, canvasW, canvasH, nodePositions, 32);
                                        }
                                    }
                                }

                                if (scoreSystem.isAlarmFull())
                                {
                                    eventLog.record("[FAILED] Alarm triggered!");
                                    string res = showResultScreen(window, false, 0, scoreSystem.alarmSteps, optimalSteps, scoreSystem.penalties, font);
                                    
                                    if (res == "Exit") 
                                        return "Exit";
                                    
                                    if (res == "Retry")
                                    {
                                        setupLevel(ctx);
                                    }
                                    else
                                    {
                                        return "CAMPAIGN MAP";
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }

            if (const auto* mr = event->getIf<Event::MouseButtonReleased>())
            {
                if (mr->button == Mouse::Button::Left)
                {
                    draggingSlider = false;
                }
            }
        }

        if (draggingSlider)
        {
            speedRatio = clampVal((mouseX - speedSlider[0]) / speedSlider[2], 0.05f, 0.95f);
            speedMultiplier = 0.5f + speedRatio * 2.5f;
        }

        background.setScale({ scaleX, scaleY });
        window.clear();
        window.draw(background);

        //lLines
        for (int i = 0; i < nodePositionCount; ++i)
        {
            int pid = nodePositions[i].parentId;
            if (pid >= 0)
            {
                int pIdx = -1;
                for (int j = 0; j < nodePositionCount; ++j)
                {
                    if (nodePositions[j].id == pid) { pIdx = j; break; }
                }

                if (pIdx >= 0)
                {
                    Vector2f p1(nodePositions[pIdx].x * scaleX, nodePositions[pIdx].y * scaleY);
                    Vector2f p2(nodePositions[i].x * scaleX, nodePositions[i].y * scaleY);
                    Color lineColor(180, 180, 180);
                    if (nodePositions[i].isPath && nodePositions[pIdx].isPath)
                        lineColor = Color(0, 220, 255);
                    else if (nodePositions[i].isVisited && nodePositions[pIdx].isVisited)
                        lineColor = Color(0, 200, 110);

                    drawThickLine(window, p1, p2, 4.f * scaleX, lineColor);
                }
            }
        }

        //nodes
        for (int i = 0; i < nodePositionCount; ++i)
        {
            const auto& pos = nodePositions[i];
            int nodeVal = pos.value;

            float r = pos.radius * scaleX;
            CircleShape circle(r);
            circle.setOrigin({ r, r });
            circle.setPosition({ pos.x * scaleX, pos.y * scaleY });

            
            circle.setFillColor(Color(0, 191, 99));
            circle.setOutlineThickness(3.5f * scaleX);
            circle.setOutlineColor(Color(25, 30, 35));

            if (pos.isGreyed)
            {
                circle.setFillColor(Color(55, 60, 65, 200));
                circle.setOutlineColor(Color(80, 85, 90));
            }
            else if (pos.isViolated)
            {
                circle.setFillColor(Color(220, 50, 50));
                circle.setOutlineColor(Color(255, 120, 120));
                circle.setOutlineThickness(5.f * scaleX);
            }
            else if (pos.isSelected || (isAct4 && selectedHeapIdx == i))
            {
                circle.setOutlineColor(Color(255, 215, 0));
                circle.setOutlineThickness(6.f * scaleX);
            }
            else if (pos.isLCA)
            {
                circle.setFillColor(Color(160, 60, 220));
                circle.setOutlineColor(Color(255, 215, 0));
                circle.setOutlineThickness(5.f * scaleX);
            }
            else if (pos.isPath)
            {
                circle.setFillColor(Color(0, 180, 220));
                circle.setOutlineColor(Color::White);
                circle.setOutlineThickness(4.f * scaleX);
            }
            else if (pos.isVisited)
            {
                circle.setFillColor(Color(0, 165, 85));
                circle.setOutlineColor(Color(180, 255, 200));
                circle.setOutlineThickness(4.f * scaleX);
            }

            //hover
            float mdx = mouseX - pos.x;
            float mdy = mouseY - pos.y;
            if (sqrt(mdx * mdx + mdy * mdy) <= pos.radius)
            {
                circle.setOutlineColor(Color(120, 255, 220));
                circle.setOutlineThickness(5.f * scaleX);
            }

            window.draw(circle);

            Text nodeText(font, to_string(nodeVal), static_cast<unsigned int>(24 * scaleX));
            nodeText.setFillColor(pos.isGreyed ? Color(130, 135, 140) : Color::White);
            auto bounds = nodeText.getLocalBounds();
            nodeText.setOrigin({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f });
            nodeText.setPosition({ pos.x * scaleX, pos.y * scaleY });
            window.draw(nodeText);

            if (!pos.badgeText.empty())
            {
                float bw = 38.f * scaleX;
                float bh = 18.f * scaleY;
                RectangleShape badge({ bw, bh });
                badge.setOrigin({ bw * 0.5f, bh * 0.5f });
                badge.setPosition({ (pos.x + pos.radius * 0.7f) * scaleX, (pos.y - pos.radius * 0.7f) * scaleY });
                badge.setFillColor(pos.badgeColor);
                window.draw(badge);

                Text bText(font, pos.badgeText, static_cast<unsigned int>(13 * scaleX));
                bText.setFillColor(Color::Black);
                auto bb = bText.getLocalBounds();
                bText.setOrigin({ bb.position.x + bb.size.x * 0.5f, bb.position.y + bb.size.y * 0.5f });
                bText.setPosition(badge.getPosition());
                window.draw(bText);
            }
        }

        for (int bIdx = 0; bIdx < levelButtonCount; ++bIdx)
        {
            const auto& btn = levelButtons[bIdx];
            bool isHovered = isInside(mouseX, mouseY, btn.x, btn.y, btn.w, btn.h);
            bool isActive = (btn.id == activeButtonId);

            if (isActive)
            {
                RectangleShape activeBox;
                activeBox.setPosition({ btn.x * scaleX, btn.y * scaleY });
                activeBox.setSize({ btn.w * scaleX, btn.h * scaleY });
                activeBox.setFillColor(Color(0, 191, 99, 45));
                activeBox.setOutlineThickness(3.f * scaleX);
                activeBox.setOutlineColor(Color(0, 191, 99, 230));
                window.draw(activeBox);
            }
            else if (isHovered)
            {
                RectangleShape hoverBox;
                hoverBox.setPosition({ btn.x * scaleX, btn.y * scaleY });
                hoverBox.setSize({ btn.w * scaleX, btn.h * scaleY });
                hoverBox.setFillColor(Color(255, 255, 255, 25));
                hoverBox.setOutlineThickness(2.5f * scaleX);
                hoverBox.setOutlineColor(Color(120, 255, 180, 200));
                window.draw(hoverBox);
            }

            if (btn.hasInput)
            {
                bool inputHovered = isInside(mouseX, mouseY, btn.ix, btn.iy, btn.iw, btn.ih);
                if (inputHovered)
                {
                    RectangleShape ibox;
                    ibox.setPosition({ btn.ix * scaleX, btn.iy * scaleY });
                    ibox.setSize({ btn.iw * scaleX, btn.ih * scaleY });
                    ibox.setFillColor(Color(255, 255, 255, 30));
                    ibox.setOutlineThickness(2.f * scaleX);
                    ibox.setOutlineColor(Color(255, 215, 0, 220));
                    window.draw(ibox);
                }
            }
        }

        if (isAct4)
        {
            for (int c = 0; c < arrayCellCount; ++c)
            {
                const auto& cell = arrayCells[c];
                float cx = cell.x * scaleX;
                float cy = cell.y * scaleY;
                float cw = cell.w * scaleX;
                float ch = cell.h * scaleY;

                bool hasVal = (static_cast<int>(c) < arraySize);
                bool inActiveHeap = (static_cast<int>(c) < activeHeapSize);
                bool isCellSelected = (selectedHeapIdx == static_cast<int>(c));
                bool isCellHovered = isInside(mouseX, mouseY, cell.x, cell.y, cell.w, cell.h);

                RectangleShape cellShape({ cw, ch });
                cellShape.setPosition({ cx, cy });

                if (hasVal)
                {
                    if (isCellSelected)
                    {
                        cellShape.setFillColor(Color(255, 215, 0, 80));
                        cellShape.setOutlineThickness(3.5f * scaleX);
                        cellShape.setOutlineColor(Color(255, 215, 0));
                    }
                    else if (inActiveHeap)
                    {
                        cellShape.setFillColor(isCellHovered ? Color(0, 191, 99, 90) : Color(0, 191, 99, 45));
                        cellShape.setOutlineThickness(2.f * scaleX);
                        cellShape.setOutlineColor(Color(0, 191, 99, 220));
                    }
                    else
                    {
                        cellShape.setFillColor(isCellHovered ? Color(90, 130, 250, 90) : Color(70, 100, 220, 50));
                        cellShape.setOutlineThickness(2.f * scaleX);
                        cellShape.setOutlineColor(Color(120, 160, 255, 230));
                    }
                }
                else
                {
                    cellShape.setFillColor(Color(20, 25, 30, 100));
                    cellShape.setOutlineThickness(1.5f * scaleX);
                    cellShape.setOutlineColor(Color(60, 70, 80, 140));
                }
                window.draw(cellShape);

                if (hasVal)
                {
                    Text valText(font, to_string(heapArray[c]), static_cast<unsigned int>(26 * scaleX));
                    valText.setFillColor(Color::White);
                    auto vb = valText.getLocalBounds();
                    valText.setOrigin({ vb.position.x + vb.size.x * 0.5f, vb.position.y + vb.size.y * 0.5f });
                    valText.setPosition({ cx + cw * 0.5f, cy + ch * 0.5f });
                    window.draw(valText);
                }

                Text idxText(font, "[" + to_string(c) + "]", static_cast<unsigned int>(14 * scaleX));
                idxText.setFillColor(inActiveHeap ? Color(160, 220, 180) : (hasVal ? Color(170, 190, 255) : Color(120, 130, 140)));
                auto ib = idxText.getLocalBounds();
                idxText.setOrigin({ ib.position.x + ib.size.x * 0.5f, 0.f });
                idxText.setPosition({ cx + cw * 0.5f, cy + ch + 3.f * scaleY });
                window.draw(idxText);
            }
        }

        Text curAlarmText(font, to_string(scoreSystem.alarmSteps), static_cast<unsigned int>(34 * scaleY));
        curAlarmText.setFillColor(scoreSystem.alarmSteps >= scoreSystem.alarmCapacity - 2 ? Color(255, 90, 90) : Color::White);
        auto caBounds = curAlarmText.getLocalBounds();
        curAlarmText.setOrigin({ caBounds.position.x + caBounds.size.x, caBounds.position.y });
        curAlarmText.setPosition({ 1075.f * scaleX, 142.f * scaleY });
        window.draw(curAlarmText);

        Text maxAlarmText(font, to_string(scoreSystem.alarmCapacity), static_cast<unsigned int>(34 * scaleY));
        maxAlarmText.setFillColor(Color::White);
        maxAlarmText.setPosition({ 1125.f * scaleX, 142.f * scaleY });
        window.draw(maxAlarmText);

        Text scoreText(font, to_string(scoreSystem.score), static_cast<unsigned int>(36 * scaleY));
        scoreText.setFillColor(Color(255, 235, 190));
        scoreText.setPosition({ 1380.f * scaleX, 148.f * scaleY });
        window.draw(scoreText);

        float logY = (eventLogY + 70.f) * scaleY;
        float logLineH = 26.f * scaleY;

        for (int i = 0; i < eventLog.count; ++i)
        {
            if (logY + logLineH > (eventLogY + eventLogH - 15.f) * scaleY) break;

            const string& msg = eventLog.messages[i];
            Text logText(font, msg, static_cast<unsigned int>(17 * scaleY));

            if (msg.find("[VISIT]") != string::npos || msg.find("[HEAP_SWAP]") != string::npos || msg.find("[EXTRACT_MAX]") != string::npos)
                logText.setFillColor(Color(0, 240, 130));
            else if (msg.find("[PENALTY]") != string::npos || msg.find("[FAILED]") != string::npos)
                logText.setFillColor(Color(255, 90, 90));
            else if (msg.find("[HINT]") != string::npos || msg.find("[VICTORY]") != string::npos || msg.find("[FOUND]") != string::npos)
                logText.setFillColor(Color(255, 215, 0));
            else if (msg.find("[MODE]") != string::npos || msg.find("[PATROL]") != string::npos || msg.find("[COMPARE]") != string::npos || msg.find("[CASE]") != string::npos)
                logText.setFillColor(Color(120, 220, 255));
            else
                logText.setFillColor(Color(210, 210, 210));

            logText.setPosition({ (eventLogX + 18.f) * scaleX, logY });
            window.draw(logText);
            logY += logLineH;
        }

        checkToolbarHover(window, mouseX, mouseY, scaleX, scaleY, undoBtn);
        checkToolbarHover(window, mouseX, mouseY, scaleX, scaleY, resetBtn);
        checkToolbarHover(window, mouseX, mouseY, scaleX, scaleY, replayBtn);
        checkToolbarHover(window, mouseX, mouseY, scaleX, scaleY, hintBtn);
        checkToolbarHover(window, mouseX, mouseY, scaleX, scaleY, backArrow);

        float sliderKnobX = (speedSlider[0] + speedRatio * speedSlider[2]) * scaleX;
        float sliderKnobY = (speedSlider[1] + speedSlider[3] * 0.5f) * scaleY;
        CircleShape knob(10.f * scaleX);
        knob.setOrigin({ 10.f * scaleX, 10.f * scaleX });
        knob.setPosition({ sliderKnobX, sliderKnobY });
        knob.setFillColor(Color(0, 191, 99));
        knob.setOutlineThickness(2.f * scaleX);
        knob.setOutlineColor(Color::White);
        window.draw(knob);

        window.display();
    }

    return "Exit";
}
