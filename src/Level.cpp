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
    if (length == 0.f) return;

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
        optimalSteps = optimal;
        alarmCapacity = static_cast<int>(ceil(1.5f * static_cast<float>(optimal))) + 2;
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
        return alarmSteps >= alarmCapacity;
    }

    int calculateStars() const
    {
        if (isAlarmFull()) return 0;

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

struct NodePosition
{
    int id;
    int value;
    float x;
    float y;
    float radius;
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
    if (!node) return;
    adjustParentCenters(node->left, positions, posCount);
    adjustParentCenters(node->right, positions, posCount);

    if (node->left && node->right)
    {
        float lx = 0.f, rx = 0.f;
        int found = 0;
        for (int i = 0; i < posCount; ++i)
        {
            if (positions[i].id == node->left->id) { lx = positions[i].x; found++; }
            if (positions[i].id == node->right->id) { rx = positions[i].x; found++; }
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

static int layoutBinaryTree(TreeNode* root, float canvasX, float canvasY, float canvasW, float canvasH,
                            NodePosition positions[], int maxPositions)
{
    if (!root) return 0;

    TreeNode* orderNodes[32];
    int orderDepths[32];
    int totalNodes = 0;
    int maxDepth = 0;

    computeInorder(root, 0, totalNodes, orderNodes, orderDepths, maxDepth);
    if (totalNodes > maxPositions) totalNodes = maxPositions;

    const float padX = 70.f;
    const float padTop = 60.f;
    const float padBottom = 70.f;

    float usableW = canvasW - 2.f * padX;
    float usableH = canvasH - padTop - padBottom;

    float xStep = (totalNodes > 1) ? (usableW / static_cast<float>(totalNodes - 1)) : 0.f;
    float yStep = (maxDepth > 0) ? (usableH / static_cast<float>(maxDepth)) : 0.f;

    float radius = 34.f;
    if (totalNodes > 8) radius = 30.f;
    if (totalNodes > 12) radius = 26.f;

    for (int i = 0; i < totalNodes; ++i)
    {
        TreeNode* n = orderNodes[i];
        float nx = (totalNodes > 1) ? (canvasX + padX + static_cast<float>(i) * xStep) : (canvasX + canvasW * 0.5f);
        float ny = canvasY + padTop + static_cast<float>(orderDepths[i]) * yStep;

        positions[i] = { n->id, n->value, nx, ny, radius };
    }

    adjustParentCenters(root, positions, totalNodes);
    return totalNodes;
}

// Result Screen Helper (Pass or Fail)
static string showResultScreen(RenderWindow& window, bool isPass, int stars, int steps, int optimalSteps,
                               int penalties, const Font& font)
{
    string bgPath = isPass ? "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\result_pass.png"
                           : "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\result_fail.png";
    Texture bgTex;
    if (!bgTex.loadFromFile(bgPath))
    {
        return "Back";
    }

    Sprite bgSprite(bgTex);

    const float starCenters[3][2] = {
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

                    // Back arrow
                    if (isInside(mx, my, 60.f, 72.f, 95.f, 80.f))
                    {
                        return "Back";
                    }

                    // Main action button (Next Level or Try Again)
                    if (isInside(mx, my, 518.f, 862.f, 884.f, 125.f))
                    {
                        return isPass ? "Next" : "Retry";
                    }
                }
            }
        }

        const Vector2u winSize = window.getSize();
        if (winSize.x == 0 || winSize.y == 0) continue;
        const float sx = static_cast<float>(winSize.x) / 1920.f;
        const float sy = static_cast<float>(winSize.y) / 1080.f;

        bgSprite.setScale({ sx, sy });

        window.clear();
        window.draw(bgSprite);

        // Draw stars
        if (isPass)
        {
            for (int s = 0; s < 3; ++s)
            {
                drawStar(window, starCenters[s][0] * sx, starCenters[s][1] * sy, 40.f * sx, stars > s);
            }
        }

        // Draw steps value
        Text stepsText(font, to_string(steps) + " / " + to_string(optimalSteps), static_cast<unsigned int>(42 * sy));
        stepsText.setFillColor(Color::White);
        stepsText.setPosition({ 1020.f * sx, 618.f * sy });
        window.draw(stepsText);

        // Draw penalties value
        Text penText(font, to_string(penalties), static_cast<unsigned int>(42 * sy));
        penText.setFillColor(penalties > 0 ? Color(255, 100, 100) : Color::White);
        penText.setPosition({ 1020.f * sx, 714.f * sy });
        window.draw(penText);

        window.display();
    }

    return "Back";
}

enum class TraversalMode
{
    PREORDER,
    INORDER,
    POSTORDER,
    LEVEL_ORDER
};

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
    for (char& c : normId) if (c == '.') c = '_';
    string dotId = levelId;
    for (char& c : dotId) if (c == '_') c = '.';

    // Generic layout defaults (1920x1080 design space)
    float canvasX = 398.f, canvasY = 265.f, canvasW = 1100.f, canvasH = 648.f;
    float eventLogX = 1523.f, eventLogY = 257.f, eventLogW = 343.f, eventLogH = 648.f;
    float alarmSlotX = 1085.f, alarmSlotY = 140.f;
    float scoreSlotX = 1360.f, scoreSlotY = 150.f;

    float undoBtn[4] = { 200.f, 955.f, 140.f, 70.f };
    float resetBtn[4] = { 518.f, 955.f, 127.f, 70.f };
    float replayBtn[4] = { 808.f, 955.f, 154.f, 70.f };
    float hintBtn[4] = { 1112.f, 955.f, 106.f, 70.f };
    float speedSlider[4] = { 1540.f, 975.f, 218.f, 30.f };
    float backArrow[4] = { 50.f, 28.f, 58.f, 44.f };

    string modeBtnIds[4] = { "preorder_traversal", "inorder_traversal", "postorder_traversal", "level_order" };
    float modeBtnX[4] = { 54.f, 54.f, 54.f, 54.f };
    float modeBtnY[4] = { 384.f, 500.f, 618.f, 739.f };
    float modeBtnW = 251.f;
    float modeBtnH = 77.f;
    int modeBtnCount = 4;

    int seed = 1101;
    int nodeCount = 7;
    string taskType = "traversal";
    string bgFileName = "level_" + normId + ".png";

    // Load layout from vault_layout.json
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
                    if (lvl.contains("buttons") && lvl["buttons"].is_array())
                    {
                        int bIdx = 0;
                        for (const auto& btn : lvl["buttons"])
                        {
                            if (bIdx >= 4) break;
                            modeBtnIds[bIdx] = btn.value("id", "");
                            modeBtnX[bIdx] = btn.value("x", 54.f);
                            modeBtnY[bIdx] = btn.value("y", 384.f);
                            modeBtnW = btn.value("w", 251.f);
                            modeBtnH = btn.value("h", 77.f);
                            bIdx++;
                        }
                        modeBtnCount = bIdx;
                    }
                }
            }
            catch (...) {}
        }
    }

    // Load level configuration from levels.json
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

    // Load background image
    string imagePath = "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\" + bgFileName;

    Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile(imagePath))
    {
        return "Can't_load_image";
    }
    Sprite background(backgroundTexture);

    // Load font
    Font font;
    if (!font.openFromFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\fonts\\perandory-semi-condensed.otf"))
    {
        font.openFromFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\fonts\\CinzelDecorative-Regular.ttf");
    }

    // Build binary tree
    BinaryTree tree = BinaryTree::generateSeeded(seed, nodeCount);

    // Compute layout
    NodePosition nodePositions[32];
    int nodePositionCount = layoutBinaryTree(tree.getRoot(), canvasX, canvasY, canvasW, canvasH, nodePositions, 32);

    // Game state
    TraversalMode currentMode = TraversalMode::PREORDER;
    TraversalResult targetSequence = tree.preorder();

    LevelScore scoreSystem;
    scoreSystem.reset(targetSequence.size);

    EventLogger eventLog;
    eventLog.record("[PATROL] Preorder route active");
    eventLog.record("[TARGET] Predict " + to_string(targetSequence.size) + " rooms");

    int visitedNodes[32];
    int visitedCount = 0;
    int visitOrderHistory[32];
    int historyCount = 0;

    float speedRatio = 0.35f;
    float speedMultiplier = 1.0f;

    bool isReplaying = false;
    int replayStep = 0;
    Clock replayClock;

    int hintNode = -1;
    Clock hintClock;

    bool draggingSlider = false;

    auto resetLevelState = [&](TraversalMode newMode) {
        currentMode = newMode;
        switch (currentMode)
        {
        case TraversalMode::PREORDER:
            targetSequence = tree.preorder();
            eventLog.record("[MODE] Switched to Preorder");
            break;
        case TraversalMode::INORDER:
            targetSequence = tree.inorder();
            eventLog.record("[MODE] Switched to Inorder");
            break;
        case TraversalMode::POSTORDER:
            targetSequence = tree.postorder();
            eventLog.record("[MODE] Switched to Postorder");
            break;
        case TraversalMode::LEVEL_ORDER:
            targetSequence = tree.levelOrder();
            eventLog.record("[MODE] Switched to Level Order");
            break;
        }

        scoreSystem.reset(targetSequence.size);
        visitedCount = 0;
        historyCount = 0;
        isReplaying = false;
        hintNode = -1;
    };

    while (window.isOpen())
    {
        const Vector2u windowSize = window.getSize();
        if (windowSize.x == 0 || windowSize.y == 0) continue;

        const float scaleX = static_cast<float>(windowSize.x) / 1920.f;
        const float scaleY = static_cast<float>(windowSize.y) / 1080.f;

        const Vector2i mousePixel = Mouse::getPosition(window);
        const float mouseX = static_cast<float>(mousePixel.x) / scaleX;
        const float mouseY = static_cast<float>(mousePixel.y) / scaleY;

        // Replay update
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

        // Hint timer
        if (hintNode != -1 && hintClock.getElapsedTime().asSeconds() > 2.5f)
        {
            hintNode = -1;
        }

        // Poll events
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
                    if (visitedCount > 0)
                    {
                        int lastVal = visitedNodes[--visitedCount];
                        if (historyCount > 0) historyCount--;
                        scoreSystem.undoStep(1);
                        eventLog.undo();
                        eventLog.record("[UNDO] Reverted room " + to_string(lastVal));
                    }
                }
                else if (kp->code == Keyboard::Key::H)
                {
                    if (visitedCount < targetSequence.size)
                    {
                        hintNode = targetSequence.get(visitedCount);
                        hintClock.restart();
                        scoreSystem.useHint();
                        eventLog.record("[HINT] Next room is " + to_string(hintNode));
                    }
                }
                else if (kp->code == Keyboard::Key::R)
                {
                    resetLevelState(currentMode);
                }
            }

            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    // 1. Back button
                    if (isInside(mouseX, mouseY, backArrow[0], backArrow[1], backArrow[2], backArrow[3]))
                    {
                        return "Back";
                    }

                    // 2. Traversal mode buttons
                    for (int i = 0; i < 4; ++i)
                    {
                        if (isInside(mouseX, mouseY, modeBtnX[i], modeBtnY[i], modeBtnW, modeBtnH))
                        {
                            if (i == 0) resetLevelState(TraversalMode::PREORDER);
                            else if (i == 1) resetLevelState(TraversalMode::INORDER);
                            else if (i == 2) resetLevelState(TraversalMode::POSTORDER);
                            else if (i == 3) resetLevelState(TraversalMode::LEVEL_ORDER);
                            break;
                        }
                    }

                    // 3. Toolbar buttons
                    if (isInside(mouseX, mouseY, undoBtn[0], undoBtn[1], undoBtn[2], undoBtn[3]))
                    {
                        if (visitedCount > 0)
                        {
                            int lastVal = visitedNodes[--visitedCount];
                            if (historyCount > 0) historyCount--;
                            scoreSystem.undoStep(1);
                            eventLog.undo();
                            eventLog.record("[UNDO] Reverted room " + to_string(lastVal));
                        }
                    }
                    else if (isInside(mouseX, mouseY, resetBtn[0], resetBtn[1], resetBtn[2], resetBtn[3]))
                    {
                        resetLevelState(currentMode);
                        eventLog.record("[RESET] Level reset");
                    }
                    else if (isInside(mouseX, mouseY, replayBtn[0], replayBtn[1], replayBtn[2], replayBtn[3]))
                    {
                        if (historyCount > 0 || visitedCount == targetSequence.size)
                        {
                            if (historyCount == 0)
                            {
                                for (int i = 0; i < visitedCount; ++i)
                                    visitOrderHistory[i] = visitedNodes[i];
                                historyCount = visitedCount;
                            }
                            visitedCount = 0;
                            isReplaying = true;
                            replayStep = 0;
                            replayClock.restart();
                            eventLog.record("[REPLAY] Playing animation");
                        }
                    }
                    else if (isInside(mouseX, mouseY, hintBtn[0], hintBtn[1], hintBtn[2], hintBtn[3]))
                    {
                        if (visitedCount < targetSequence.size)
                        {
                            hintNode = targetSequence.get(visitedCount);
                            hintClock.restart();
                            scoreSystem.useHint();
                            eventLog.record("[HINT] Next room is " + to_string(hintNode));
                        }
                    }
                    else if (isInside(mouseX, mouseY, speedSlider[0], speedSlider[1], speedSlider[2], speedSlider[3]))
                    {
                        draggingSlider = true;
                        speedRatio = clampVal((mouseX - speedSlider[0]) / speedSlider[2], 0.05f, 0.95f);
                        speedMultiplier = 0.5f + speedRatio * 2.5f;
                    }

                    // 4. Node clicks in diagram
                    if (!isReplaying && visitedCount < targetSequence.size)
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

                                // Check if already visited
                                bool already = false;
                                for (int v = 0; v < visitedCount; ++v)
                                {
                                    if (visitedNodes[v] == nodeVal) { already = true; break; }
                                }
                                if (already)
                                {
                                    eventLog.record("Room " + to_string(nodeVal) + " already visited");
                                    break;
                                }

                                int expectedVal = targetSequence.get(visitedCount);
                                if (nodeVal == expectedVal)
                                {
                                    // Correct visit
                                    visitedNodes[visitedCount++] = nodeVal;
                                    visitOrderHistory[historyCount++] = nodeVal;
                                    scoreSystem.addStep();
                                    eventLog.record("[VISIT] Room " + to_string(nodeVal) +
                                                   " (" + to_string(visitedCount) + "/" + to_string(targetSequence.size) + ")");

                                    // Check win condition
                                    if (visitedCount == targetSequence.size)
                                    {
                                        eventLog.record("[VICTORY] Vault unlocked!");
                                        int stars = scoreSystem.calculateStars();
                                        recordResult(progress, levelId, stars);

                                        string res = showResultScreen(window, true, stars, scoreSystem.alarmSteps,
                                                                      targetSequence.size, scoreSystem.penalties, font);
                                        if (res == "Exit") return "Exit";
                                        return "CAMPAIGN MAP";
                                    }
                                }
                                else
                                {
                                    // Wrong click penalty (+1 alarm)
                                    scoreSystem.addPenalty(1);
                                    eventLog.record("[PENALTY] Wrong room " + to_string(nodeVal) +
                                                   ", expected " + to_string(expectedVal) + " (+1 Alarm)");

                                    // Check fail condition
                                    if (scoreSystem.isAlarmFull())
                                    {
                                        eventLog.record("[FAILED] Alarm triggered!");
                                        string res = showResultScreen(window, false, 0, scoreSystem.alarmSteps,
                                                                      targetSequence.size, scoreSystem.penalties, font);
                                        if (res == "Exit") return "Exit";
                                        if (res == "Retry")
                                        {
                                            resetLevelState(currentMode);
                                        }
                                        else
                                        {
                                            return "CAMPAIGN MAP";
                                        }
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

        // Slider drag update
        if (draggingSlider)
        {
            speedRatio = clampVal((mouseX - speedSlider[0]) / speedSlider[2], 0.05f, 0.95f);
            speedMultiplier = 0.5f + speedRatio * 2.5f;
        }

        // ---------------- Rendering ----------------
        background.setScale({ scaleX, scaleY });
        window.clear();
        window.draw(background);

        // 1. Draw Connecting Lines (color: #b4b4b4)
        for (int i = 0; i < nodePositionCount; ++i)
        {
            TreeNode* n = tree.findNode(nodePositions[i].value);
            if (!n) continue;

            if (n->left)
            {
                for (int j = 0; j < nodePositionCount; ++j)
                {
                    if (nodePositions[j].value == n->left->value)
                    {
                        Vector2f p1(nodePositions[i].x * scaleX, nodePositions[i].y * scaleY);
                        Vector2f p2(nodePositions[j].x * scaleX, nodePositions[j].y * scaleY);
                        drawThickLine(window, p1, p2, 4.f * scaleX, Color(180, 180, 180));
                        break;
                    }
                }
            }
            if (n->right)
            {
                for (int j = 0; j < nodePositionCount; ++j)
                {
                    if (nodePositions[j].value == n->right->value)
                    {
                        Vector2f p1(nodePositions[i].x * scaleX, nodePositions[i].y * scaleY);
                        Vector2f p2(nodePositions[j].x * scaleX, nodePositions[j].y * scaleY);
                        drawThickLine(window, p1, p2, 4.f * scaleX, Color(180, 180, 180));
                        break;
                    }
                }
            }
        }

        // 2. Draw Nodes (color: #00bf63, dark outline, white text)
        for (int i = 0; i < nodePositionCount; ++i)
        {
            const auto& pos = nodePositions[i];
            int nodeVal = pos.value;

            bool isNodeVisited = false;
            int visitIdx = -1;
            for (int v = 0; v < visitedCount; ++v)
            {
                if (visitedNodes[v] == nodeVal)
                {
                    isNodeVisited = true;
                    visitIdx = v + 1;
                    break;
                }
            }

            bool isHinted = (nodeVal == hintNode);

            float r = pos.radius * scaleX;
            CircleShape circle(r);
            circle.setOrigin({ r, r });
            circle.setPosition({ pos.x * scaleX, pos.y * scaleY });

            // Base color: #00bf63 (RGB: 0, 191, 99)
            circle.setFillColor(Color(0, 191, 99));
            circle.setOutlineThickness(3.5f * scaleX);
            circle.setOutlineColor(Color(25, 30, 35));

            if (isHinted)
            {
                circle.setOutlineColor(Color(255, 215, 0)); // Gold outline for hint
                circle.setOutlineThickness(6.f * scaleX);
            }
            else if (isNodeVisited)
            {
                circle.setFillColor(Color(0, 165, 85));
                circle.setOutlineColor(Color(180, 255, 200));
                circle.setOutlineThickness(4.f * scaleX);
            }

            // Hover outline
            float mdx = mouseX - pos.x;
            float mdy = mouseY - pos.y;
            if (sqrt(mdx * mdx + mdy * mdy) <= pos.radius)
            {
                circle.setOutlineColor(Color(120, 255, 220));
                circle.setOutlineThickness(5.f * scaleX);
            }

            window.draw(circle);

            // Data inside node: white text
            Text nodeText(font, to_string(nodeVal), static_cast<unsigned int>(26 * scaleX));
            nodeText.setFillColor(Color::White);
            auto bounds = nodeText.getLocalBounds();
            nodeText.setOrigin({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f });
            nodeText.setPosition({ pos.x * scaleX, pos.y * scaleY });
            window.draw(nodeText);

            // Visit sequence badge
            if (isNodeVisited)
            {
                float br = 10.f * scaleX;
                CircleShape badge(br);
                badge.setOrigin({ br, br });
                badge.setPosition({ (pos.x + pos.radius * 0.7f) * scaleX, (pos.y - pos.radius * 0.7f) * scaleY });
                badge.setFillColor(Color(255, 215, 0));
                window.draw(badge);

                Text badgeText(font, to_string(visitIdx), static_cast<unsigned int>(14 * scaleX));
                badgeText.setFillColor(Color::Black);
                auto bb = badgeText.getLocalBounds();
                badgeText.setOrigin({ bb.position.x + bb.size.x * 0.5f, bb.position.y + bb.size.y * 0.5f });
                badgeText.setPosition(badge.getPosition());
                window.draw(badgeText);
            }
        }

        // 3. Highlight Active Traversal Mode Button (Left Panel)
        int activeIdx = static_cast<int>(currentMode);
        for (int i = 0; i < 4; ++i)
        {
            if (i == activeIdx)
            {
                RectangleShape activeBox;
                activeBox.setPosition({ modeBtnX[i] * scaleX, modeBtnY[i] * scaleY });
                activeBox.setSize({ modeBtnW * scaleX, modeBtnH * scaleY });
                activeBox.setFillColor(Color(0, 191, 99, 40));
                activeBox.setOutlineThickness(3.f * scaleX);
                activeBox.setOutlineColor(Color(0, 191, 99, 230));
                window.draw(activeBox);
            }
            else if (isInside(mouseX, mouseY, modeBtnX[i], modeBtnY[i], modeBtnW, modeBtnH))
            {
                RectangleShape hoverBox;
                hoverBox.setPosition({ modeBtnX[i] * scaleX, modeBtnY[i] * scaleY });
                hoverBox.setSize({ modeBtnW * scaleX, modeBtnH * scaleY });
                hoverBox.setFillColor(Color(255, 255, 255, 20));
                hoverBox.setOutlineThickness(2.f * scaleX);
                hoverBox.setOutlineColor(Color(120, 255, 180, 160));
                window.draw(hoverBox);
            }
        }

        // 4. Fill Empty Fields:
        // A. Alarm Meter: current / capacity around alarm_slot (x: 1085, y: 140)
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

        // B. Score in score_slot (x: 1360, y: 150)
        Text scoreText(font, to_string(scoreSystem.score), static_cast<unsigned int>(36 * scaleY));
        scoreText.setFillColor(Color(255, 235, 190));
        scoreText.setPosition({ 1380.f * scaleX, 148.f * scaleY });
        window.draw(scoreText);

        // C. Events Log in event_log panel (x: 1523, y: 257, w: 343, h: 648)
        float logY = (eventLogY + 18.f) * scaleY;
        float logLineH = 26.f * scaleY;

        for (int i = 0; i < eventLog.count; ++i)
        {
            if (logY + logLineH > (eventLogY + eventLogH - 10.f) * scaleY) break;

            const string& msg = eventLog.messages[i];
            Text logText(font, msg, static_cast<unsigned int>(18 * scaleY));

            if (msg.find("[VISIT]") != string::npos)
                logText.setFillColor(Color(0, 240, 130));
            else if (msg.find("[PENALTY]") != string::npos || msg.find("[FAILED]") != string::npos)
                logText.setFillColor(Color(255, 90, 90));
            else if (msg.find("[HINT]") != string::npos || msg.find("[VICTORY]") != string::npos)
                logText.setFillColor(Color(255, 215, 0));
            else if (msg.find("[MODE]") != string::npos || msg.find("[PATROL]") != string::npos)
                logText.setFillColor(Color(120, 220, 255));
            else
                logText.setFillColor(Color(210, 210, 210));

            logText.setPosition({ (eventLogX + 18.f) * scaleX, logY });
            window.draw(logText);
            logY += logLineH;
        }

        // 5. Toolbar button hover feedback & Speed Slider knob
        auto checkToolbarHover = [&](const float r[4]) {
            if (isInside(mouseX, mouseY, r[0], r[1], r[2], r[3]))
            {
                RectangleShape hBox;
                hBox.setPosition({ r[0] * scaleX, r[1] * scaleY });
                hBox.setSize({ r[2] * scaleX, r[3] * scaleY });
                hBox.setFillColor(Color(255, 255, 255, 25));
                hBox.setOutlineThickness(2.f * scaleX);
                hBox.setOutlineColor(Color(0, 191, 99, 180));
                window.draw(hBox);
            }
        };

        checkToolbarHover(undoBtn);
        checkToolbarHover(resetBtn);
        checkToolbarHover(replayBtn);
        checkToolbarHover(hintBtn);

        // Speed slider knob
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
