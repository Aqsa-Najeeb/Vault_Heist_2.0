#include "GameModes.h"
#include "BinaryTree.h"
#include <SFML/Audio.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <cmath>

using namespace std;
using namespace sf;
using json = nlohmann::json;

static void safeCopyStr(char* dst, const string& src, size_t maxLen)
{
    size_t i = 0;
    while (i + 1 < maxLen && i < src.size())
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static int minInt(int a, int b) 
{ 
    return (a < b) ? a : b; 
}

static int maxInt(int a, int b) 
{ 
    return (a > b) ? a : b; 
}

static float minFloat(float a, float b) 
{ 
    return (a < b) ? a : b; 
}

static float maxFloat(float a, float b) 
{ 
    return (a > b) ? a : b; 
}

static float clampFloat(float v, float lo, float hi)
{
    if (v < lo) 
        return lo;
    
    if (v > hi) 
        return hi;
    
    return v;
}

static bool isInside(float x, float y, float left, float top, float width, float height)
{
    return (x >= left && x <= left + width && y >= top && y <= top + height);
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

static void drawCenteredText(RenderWindow& window, Text& text, float cx, float cy)
{
    FloatRect b = text.getLocalBounds();
    text.setOrigin({ b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f });
    text.setPosition({ cx, cy });
    window.draw(text);
}


static Music g_bgMusic;
static Music g_alarmMusic;
static bool g_bgLoaded = false;
static bool g_alarmLoaded = false;
static GameSettings g_settings;

static string resolveFilePath(const char* relPath, const char* altRelPath = nullptr)
{
    ifstream f(relPath);
   
    if (f.good()) 
        return relPath;
    
    if (altRelPath)
    {
        ifstream f2(altRelPath);
        
        if (f2.good()) 
            return altRelPath;
    }
    return relPath;
}

void initGameAudio()
{
    string bgPath = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\audio\\bg.mp3", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\audio\\bg.mp3");
    
    if (g_bgMusic.openFromFile(bgPath))
    {
        g_bgLoaded = true;
        g_bgMusic.setLooping(true);
        g_bgMusic.setVolume(50.f);
    
        if (g_settings.musicEnabled)
        {
            g_bgMusic.play();
        }
    }

    string alarmPath = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\audio\\alarm.mp3", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\audio\\alarm.mp3");
    
    if (g_alarmMusic.openFromFile(alarmPath))
    {
        g_alarmLoaded = true;
        g_alarmMusic.setLooping(true);
        g_alarmMusic.setVolume(85.f);
    }
}

void setGameMusicEnabled(bool enabled)
{
    g_settings.musicEnabled = enabled;
    
    if (g_bgLoaded)
    {
        if (enabled)
        {
            if (g_bgMusic.getStatus() != Music::Status::Playing)
                g_bgMusic.play();
        }
        else
        {
            g_bgMusic.stop();
        }
    }
}

void setGameAlarmEnabled(bool enabled)
{
    g_settings.alarmSoundEnabled = enabled;
}

void triggerAlarm3Seconds(RenderWindow& window, const Font* font)
{
    if (g_alarmLoaded && g_settings.alarmSoundEnabled)
    {
        g_alarmMusic.stop();
        g_alarmMusic.play();
    }

    Clock timer;
    const float duration = 3.0f;

    while (window.isOpen() && timer.getElapsedTime().asSeconds() < duration)
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
            {
                if (g_alarmLoaded) g_alarmMusic.stop();
                window.close();
                return;
            }
        }

        float elapsed = timer.getElapsedTime().asSeconds();
        float remaining = duration - elapsed;
        if (remaining < 0.f) remaining = 0.f;

        float pulse = (sin(elapsed * 12.0f) + 1.0f) * 0.5f;
        Color overlayColor(255, 0, 0, static_cast<std::uint8_t>(40 + pulse * 90));

        Vector2u winSize = window.getSize();
        RectangleShape flash({ static_cast<float>(winSize.x), static_cast<float>(winSize.y) });
        flash.setFillColor(overlayColor);
        window.draw(flash);

        if (font)
        {
            float scaleY = static_cast<float>(winSize.y) / 1080.f;
            float scaleX = static_cast<float>(winSize.x) / 1920.f;

            RectangleShape banner({ 600.f * scaleX, 100.f * scaleY });
            banner.setOrigin({ 300.f * scaleX, 50.f * scaleY });
            banner.setPosition({ winSize.x * 0.5f, winSize.y * 0.45f });
            banner.setFillColor(Color(20, 0, 0, 230));
            banner.setOutlineThickness(3.f * scaleX);
            banner.setOutlineColor(Color(255, 50, 50, 240));
            window.draw(banner);

            Text alertText(*font, "! SECURITY ALARM TRIGGERED !", static_cast<unsigned int>(28 * scaleY));
            alertText.setFillColor(Color(255, 80, 80));
            FloatRect bounds = alertText.getLocalBounds();
            alertText.setOrigin({ bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f });
            alertText.setPosition({ winSize.x * 0.5f, winSize.y * 0.43f });
            window.draw(alertText);

            int remW = static_cast<int>(remaining);
            int remF = static_cast<int>((remaining - static_cast<float>(remW)) * 10.f + 0.5f);
            if (remF >= 10) { remW++; remF -= 10; }
            if (remF < 0) remF = 0;
            string buf = "Breach alert active: " + to_string(remW) + "." + to_string(remF) + "s";
            Text timerText(*font, buf, static_cast<unsigned int>(20 * scaleY));
            timerText.setFillColor(Color(255, 200, 200));
            FloatRect tbounds = timerText.getLocalBounds();
            timerText.setOrigin({ tbounds.position.x + tbounds.size.x * 0.5f, tbounds.position.y + tbounds.size.y * 0.5f });
            timerText.setPosition({ winSize.x * 0.5f, winSize.y * 0.475f });
            window.draw(timerText);
        }

        window.display();
        sleep(milliseconds(16));
    }

    if (g_alarmLoaded)
    {
        g_alarmMusic.stop();
    }
}




GameSettings& getGameSettings()
{
    return g_settings;
}

static string getSettingsPath()
{
    return resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\data\\settings.json", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\data\\settings.json");
}

void loadGameSettings()
{
    string path = getSettingsPath();
    ifstream file(path);
    if (!file.is_open()) return;

    try
    {
        json j;
        file >> j;
        g_settings.musicEnabled = j.value("musicEnabled", true);
        g_settings.unlockAll = j.value("unlockAll", false);
        g_settings.alarmSoundEnabled = j.value("alarmSoundEnabled", true);
    }
    catch (...) {}
}

void saveGameSettings()
{
    string path = getSettingsPath();
    ofstream file(path);
    if (!file.is_open()) return;

    try
    {
        json j;
        j["musicEnabled"] = g_settings.musicEnabled;
        j["unlockAll"] = g_settings.unlockAll;
        j["alarmSoundEnabled"] = g_settings.alarmSoundEnabled;
        file << j.dump(4);
    }
    catch (...) {}
}

string runSettings(RenderWindow& window, const Font& font)
{
    Texture bgTex;
    string imgPath = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Settings.png", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Settings.png");
    
    if (!bgTex.loadFromFile(imgPath)) 
        return "MENU";

    Sprite bg(bgTex);

    const float switchX[3] = { 711.f, 1074.f, 926.f };
    const float switchY[3] = { 387.f, 531.f, 677.f };
    const float switchW[3] = { 188.f, 176.f, 189.f };
    const float switchH[3] = { 88.f,  89.f,  88.f };

    int hovered = -1;

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
            {
                window.close();
                return "EXIT GAME";
            }

            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape)
                {
                    saveGameSettings();
                    return "MENU";
                }
            }

            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    Vector2u winSize = window.getSize();
                    Vector2u imgSize = bgTex.getSize();
                    float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
                    float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
                    Vector2i mpos = Mouse::getPosition(window);
                    float mx = static_cast<float>(mpos.x) / sx;
                    float my = static_cast<float>(mpos.y) / sy;

                    if (isInside(mx, my, 25.f, 20.f, 80.f, 70.f))
                    {
                        saveGameSettings();
                        return "MENU";
                    }

                    if (isInside(mx, my, switchX[0], switchY[0], switchW[0], switchH[0]))
                    {
                        setGameMusicEnabled(!g_settings.musicEnabled);
                        saveGameSettings();
                    }
                    else if (isInside(mx, my, switchX[1], switchY[1], switchW[1], switchH[1]))
                    {
                        g_settings.unlockAll = !g_settings.unlockAll;
                        saveGameSettings();
                    }
                    else if (isInside(mx, my, switchX[2], switchY[2], switchW[2], switchH[2]))
                    {
                        setGameAlarmEnabled(!g_settings.alarmSoundEnabled);
                        saveGameSettings();
                    }
                }
            }
        }

        Vector2u winSize = window.getSize();
        Vector2u imgSize = bgTex.getSize();
        
        if (winSize.x == 0 || winSize.y == 0) 
            continue;

        float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
        float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
        bg.setScale({ sx, sy });

        Vector2i mpos = Mouse::getPosition(window);
        float mx = static_cast<float>(mpos.x) / sx;
        float my = static_cast<float>(mpos.y) / sy;

        hovered = -1;
        for (int i = 0; i < 3; ++i)
        {
            if (isInside(mx, my, switchX[i], switchY[i], switchW[i], switchH[i]))
            {
                hovered = i;
                break;
            }
        }

        window.clear();
        window.draw(bg);

        bool states[3] = { g_settings.musicEnabled, g_settings.unlockAll, g_settings.alarmSoundEnabled };

        for (int i = 0; i < 3; ++i)
        {
            float px = switchX[i] * sx;
            float py = switchY[i] * sy;
            float pw = switchW[i] * sx;
            float ph = switchH[i] * sy;

            RectangleShape pill({ pw, ph });
            pill.setPosition({ px, py });
            pill.setFillColor(states[i] ? Color(20, 60, 50, 220) : Color(40, 45, 60, 220));
            pill.setOutlineThickness(2.f * sx);
            pill.setOutlineColor(states[i] ? Color(78, 255, 152, 230) : Color(120, 130, 150, 180));
            window.draw(pill);

            float knobRadius = (ph * 0.38f);
            CircleShape knob(knobRadius);
            knob.setOrigin({ knobRadius, knobRadius });
            float knobX = states[i] ? (px + pw - knobRadius - 12.f * sx) : (px + knobRadius + 12.f * sx);
            float knobY = py + ph * 0.5f;
            knob.setPosition({ knobX, knobY });
            knob.setFillColor(states[i] ? Color(78, 255, 152) : Color(180, 190, 205));
            window.draw(knob);

            Text statusText(font, states[i] ? "ON" : "OFF", static_cast<unsigned int>(26 * sy));
            statusText.setFillColor(states[i] ? Color(78, 255, 152) : Color(160, 170, 185));
            FloatRect tb = statusText.getLocalBounds();
            statusText.setOrigin({ tb.position.x + tb.size.x * 0.5f, tb.position.y + tb.size.y * 0.5f });
            float labelX = states[i] ? (px + pw * 0.35f) : (px + pw * 0.65f);
            statusText.setPosition({ labelX, py + ph * 0.5f });
            window.draw(statusText);

            if (hovered == i)
            {
                RectangleShape hl({ pw + 6.f * sx, ph + 6.f * sy });
                hl.setOrigin({ 3.f * sx, 3.f * sy });
                hl.setPosition({ px, py });
                hl.setFillColor(Color::Transparent);
                hl.setOutlineThickness(2.f * sx);
                hl.setOutlineColor(Color(255, 255, 255, 200));
                window.draw(hl);
            }
        }

        window.display();
    }

    return "MENU";
}




struct LBEntry
{
    int attemptNumber;
    int score;
    float survivalTime;
};

static LBEntry g_lbEntries[128];
static int g_lbCount = 0;
static int g_totalAttempts = 0;

static string getLBPath()
{
    return resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\data\\leaderboard.json", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\data\\leaderboard.json");
}

static void sortLB()
{
    for (int i = 0; i < g_lbCount - 1; ++i)
    {
        for (int j = 0; j < g_lbCount - i - 1; ++j)
        {
            bool swapNeeded = false;
            if (g_lbEntries[j].score < g_lbEntries[j + 1].score)
            {
                swapNeeded = true;
            }
            else if (g_lbEntries[j].score == g_lbEntries[j + 1].score && g_lbEntries[j].survivalTime < g_lbEntries[j + 1].survivalTime)
            {
                swapNeeded = true;
            }

            if (swapNeeded)
            {
                LBEntry temp = g_lbEntries[j];
                g_lbEntries[j] = g_lbEntries[j + 1];
                g_lbEntries[j + 1] = temp;
            }
        }
    }
}

static void saveLB()
{
    string path = getLBPath();
    ofstream file(path);
    if (!file.is_open()) return;

    try
    {
        json j;
        j["totalAttempts"] = g_totalAttempts;
        json arr = json::array();
        for (int i = 0; i < g_lbCount; ++i)
        {
            json item;
            item["attempt"] = g_lbEntries[i].attemptNumber;
            item["score"] = g_lbEntries[i].score;
            item["survivalTime"] = g_lbEntries[i].survivalTime;
            arr.push_back(item);
        }
        j["attempts"] = arr;
        file << j.dump(4);
    }
    catch (...) {}
}

static void loadLB()
{
    g_lbCount = 0;
    g_totalAttempts = 0;
    string path = getLBPath();
    ifstream file(path);
    if (!file.is_open())
    {
        return;
    }

    try
    {
        json j;
        file >> j;
        if (j.is_object() && j.contains("totalAttempts"))
        {
            g_totalAttempts = j.value("totalAttempts", 0);
        }
        json arr = j.is_array() ? j : (j.contains("attempts") ? j["attempts"] : json::array());
        for (const auto& item : arr)
        {
            if (g_lbCount < 128)
            {
                g_lbEntries[g_lbCount].attemptNumber = item.value("attempt", g_lbCount + 1);
                g_lbEntries[g_lbCount].score = item.value("score", 0);
                g_lbEntries[g_lbCount].survivalTime = item.value("survivalTime", 0.0f);
                if (g_lbEntries[g_lbCount].attemptNumber > g_totalAttempts)
                {
                    g_totalAttempts = g_lbEntries[g_lbCount].attemptNumber;
                }
                g_lbCount++;
            }
        }
    }
    catch (...) {}

    sortLB();
}

static void addLBScore(const string&, int score, float survivalTime)
{
    loadLB();
    g_totalAttempts++;
    if (g_lbCount < 128)
    {
        g_lbEntries[g_lbCount].attemptNumber = g_totalAttempts;
        g_lbEntries[g_lbCount].score = score;
        g_lbEntries[g_lbCount].survivalTime = survivalTime;
        g_lbCount++;
    }
    else
    {
        if (score > g_lbEntries[127].score)
        {
            g_lbEntries[127].attemptNumber = g_totalAttempts;
            g_lbEntries[127].score = score;
            g_lbEntries[127].survivalTime = survivalTime;
        }
    }
    sortLB();
    saveLB();
}

static string formatTimer(float seconds)
{
    int totalSec = static_cast<int>(seconds);
    int mins = totalSec / 60;
    int secs = totalSec % 60;
    int ms = static_cast<int>((seconds - static_cast<float>(totalSec)) * 10.0f);
    string sMins = (mins < 10 ? "0" : "") + to_string(mins);
    string sSecs = (secs < 10 ? "0" : "") + to_string(secs);
    return sMins + ":" + sSecs + "." + to_string(ms);
}

string runLeaderboard(RenderWindow& window, const Font& font)
{
    Texture bgTex;
    string imgPath = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Leaderboard.png", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Leaderboard.png");
    
    if (!bgTex.loadFromFile(imgPath)) 
        return "MENU";

    Sprite bg(bgTex);
    loadLB();

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
            {
                window.close();
                return "EXIT GAME";
            }

            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape)
                    return "MENU";
            }

            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    Vector2u winSize = window.getSize();
                    Vector2u imgSize = bgTex.getSize();
                    float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
                    float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
                    Vector2i mpos = Mouse::getPosition(window);
                    float mx = static_cast<float>(mpos.x) / sx;
                    float my = static_cast<float>(mpos.y) / sy;

                    if (isInside(mx, my, 25.f, 20.f, 80.f, 70.f))
                        return "MENU";
                }
            }
        }

        Vector2u winSize = window.getSize();
        Vector2u imgSize = bgTex.getSize();
        if (winSize.x == 0 || winSize.y == 0) continue;

        float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
        float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
        bg.setScale({ sx, sy });

        window.clear();
        window.draw(bg);

        RectangleShape attemptHeaderBox({ 260.f * sx, 42.f * sy });
        attemptHeaderBox.setOrigin({ 130.f * sx, 21.f * sy });
        attemptHeaderBox.setPosition({ 620.f * sx, 290.f * sy });
        attemptHeaderBox.setFillColor(Color(65, 75, 140));
        attemptHeaderBox.setOutlineThickness(1.5f * sx);
        attemptHeaderBox.setOutlineColor(Color(100, 120, 190));
        window.draw(attemptHeaderBox);

        Text attemptHeader(font, "ATTEMPT", static_cast<unsigned int>(24 * sy));
        attemptHeader.setFillColor(Color(220, 235, 255));
        drawCenteredText(window, attemptHeader, 620.f * sx, 290.f * sy);

        const float startY = 350.f;
        const float rowSpacing = 58.f;
        int maxRows = minInt(10, g_lbCount);

        for (int i = 0; i < maxRows; ++i)
        {
            float cy = (startY + static_cast<float>(i) * rowSpacing + rowSpacing * 0.5f) * sy;

            Color rowColor = Color(220, 230, 245);
            if (i == 0) rowColor = Color(255, 215, 0);
            else if (i == 1) rowColor = Color(210, 225, 240);
            else if (i == 2) rowColor = Color(205, 140, 90);

            Text rankText(font, "#" + to_string(i + 1), static_cast<unsigned int>(24 * sy));
            rankText.setFillColor(rowColor);
            drawCenteredText(window, rankText, 280.f * sx, cy);

            string attemptStr = "Attempt #" + to_string(g_lbEntries[i].attemptNumber);
            Text attemptText(font, attemptStr, static_cast<unsigned int>(24 * sy));
            attemptText.setFillColor(rowColor);
            drawCenteredText(window, attemptText, 620.f * sx, cy);

            Text scoreText(font, to_string(g_lbEntries[i].score), static_cast<unsigned int>(24 * sy));
            scoreText.setFillColor(rowColor);
            drawCenteredText(window, scoreText, 1090.f * sx, cy);

            Text timeText(font, formatTimer(g_lbEntries[i].survivalTime), static_cast<unsigned int>(24 * sy));
            timeText.setFillColor(rowColor);
            drawCenteredText(window, timeText, 1440.f * sx, cy);
        }

        if (g_lbCount == 0)
        {
            Text emptyText(font, "No attempts recorded yet. Play Endless Mode to set a record!", static_cast<unsigned int>(24 * sy));
            emptyText.setFillColor(Color(180, 190, 205));
            drawCenteredText(window, emptyText, 960.f * sx, 450.f * sy);
        }

        window.display();
    }

    return "MENU";
}




struct GameNodePos
{
    int id;
    int value;
    float x;
    float y;
    float radius;
    int parentId;
    int leftId;
    int rightId;
    char badge[8];
    Color badgeColor;
    bool isSelected;
    bool isHighlighted;
    bool isGreyed;
    bool isViolated;
};

static void computeInorderRec(TreeNode* node, int depth, int& count, TreeNode* orderNodes[], int orderDepths[], int& maxDepth)
{
    if (!node || count >= 64) 
        return;
    
    if (depth > maxDepth) 
        maxDepth = depth;
    
    computeInorderRec(node->left, depth + 1, count, orderNodes, orderDepths, maxDepth);
    orderDepths[count] = depth;
    orderNodes[count++] = node;
    computeInorderRec(node->right, depth + 1, count, orderNodes, orderDepths, maxDepth);
}

static void adjustParents(TreeNode* node, GameNodePos positions[], int posCount)
{
    if (!node) 
        return;
    
    adjustParents(node->left, positions, posCount);
    adjustParents(node->right, positions, posCount);

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
                rx = positions[i].x; found++; 
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

static int layoutGeneralTree(TreeNode* root, float cx, float cy, float cw, float ch, GameNodePos positions[], int maxPos)
{
    if (!root) 
        return 0;

    TreeNode* orderNodes[64];
    int orderDepths[64];
    int total = 0;
    int maxDepth = 0;

    computeInorderRec(root, 0, total, orderNodes, orderDepths, maxDepth);
    
    if (total > maxPos) 
        total = maxPos;

    float radius = (total > 14) ? 20.f : ((total > 8) ? 24.f : 28.f);
    float padX = radius + 35.f;
    float padTop = radius + 35.f;
    float padBottom = radius + 30.f;
    float usableW = cw - 2.f * padX;
    float usableH = ch - padTop - padBottom;

    float xStep = (total > 1) ? (usableW / static_cast<float>(total - 1)) : 0.f;
    float yStep = (maxDepth > 0) ? (usableH / static_cast<float>(maxDepth)) : 0.f;

    for (int i = 0; i < total; ++i)
    {
        TreeNode* n = orderNodes[i];
        float nx = (total > 1) ? (cx + padX + static_cast<float>(i) * xStep) : (cx + cw * 0.5f);
        float ny = cy + padTop + static_cast<float>(orderDepths[i]) * yStep;

        positions[i].id = n->id;
        positions[i].value = n->value;
        positions[i].x = nx;
        positions[i].y = ny;
        positions[i].radius = radius;
        positions[i].parentId = n->parent ? n->parent->id : -1;
        positions[i].leftId = n->left ? n->left->id : -1;
        positions[i].rightId = n->right ? n->right->id : -1;
        positions[i].badge[0] = '\0';
        positions[i].badgeColor = Color(78, 255, 152);
        positions[i].isSelected = false;
        positions[i].isHighlighted = false;
        positions[i].isGreyed = false;
        positions[i].isViolated = false;
    }

    adjustParents(root, positions, total);

    
    float minX = cx + radius + 15.f;
    float maxX = cx + cw - radius - 28.f;
    float minY = cy + radius + 25.f;
    float maxY = cy + ch - radius - 15.f;

    for (int i = 0; i < total; ++i)
    {
        positions[i].x = clampFloat(positions[i].x, minX, maxX);
        positions[i].y = clampFloat(positions[i].y, minY, maxY);
    }

    return total;
}




struct ScratchAVLNode
{
    int id;
    int value;
    ScratchAVLNode* left;
    ScratchAVLNode* right;
    ScratchAVLNode* parent;
    int height;
    int size;

    ScratchAVLNode(int id = 0, int val = 0, ScratchAVLNode* p = nullptr) : id(id), value(val), left(nullptr), right(nullptr), parent(p), height(1), size(1) {}
};

class ScratchAVL
{
public:
    ScratchAVLNode* root;
    int nextId;

    ScratchAVL() : root(nullptr), nextId(1) {}
    ~ScratchAVL() { clear(); }

    void destroy(ScratchAVLNode* n)
    {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    void clear()
    {
        destroy(root);
        root = nullptr;
        nextId = 1;
    }

    int getHeight(ScratchAVLNode* n) const 
    { 
        return n ? n->height : 0; 
    }

    int getSize(ScratchAVLNode* n) const 
    { 
        return n ? n->size : 0; 
    }

    int getBF(ScratchAVLNode* n) const 
    { 
        return n ? (getHeight(n->left) - getHeight(n->right)) : 0; 
    }

    void update(ScratchAVLNode* n)
    {
        if (!n) 
            return;
        
        n->height = 1 + maxInt(getHeight(n->left), getHeight(n->right));
        n->size = 1 + getSize(n->left) + getSize(n->right);
    }

    ScratchAVLNode* rotRight(ScratchAVLNode* y)
    {
        ScratchAVLNode* x = y->left;
        
        if (!x) 
            return y;
        
        ScratchAVLNode* T2 = x->right;
        x->right = y;
        x->parent = y->parent;
        y->parent = x;
        y->left = T2;
        
        if (T2) 
            T2->parent = y;
        
        update(y);
        update(x);
        return x;
    }

    ScratchAVLNode* rotLeft(ScratchAVLNode* x)
    {
        ScratchAVLNode* y = x->right;
        
        if (!y) 
            return x;
        
        ScratchAVLNode* T2 = y->left;
        y->left = x;
        y->parent = x->parent;
        x->parent = y;
        x->right = T2;
        
        if (T2) 
            T2->parent = x;
        update(x);
        update(y);
        return y;
    }

    ScratchAVLNode* rebalance(ScratchAVLNode* n)
    {
        if (!n) 
            return nullptr;
        
        update(n);
        int bf = getBF(n);
        
        if (bf > 1)
        {
            if (getBF(n->left) < 0)
            {
                n->left = rotLeft(n->left);
                if (n->left) n->left->parent = n;
            }
            return rotRight(n);
        }
        
        if (bf < -1)
        {
            if (getBF(n->right) > 0)
            {
                n->right = rotRight(n->right);
                if (n->right) n->right->parent = n;
            }
            return rotLeft(n);
        }
        return n;
    }

    ScratchAVLNode* insertRec(ScratchAVLNode* n, ScratchAVLNode* p, int val, bool& ok, bool autoBal)
    {
        if (!n)
        {
            ok = true;
            return new ScratchAVLNode(nextId++, val, p);
        }
        
        if (val == n->value) 
        { 
            ok = false; 
            return n; 
        }
        
        if (val < n->value)
        {
            n->left = insertRec(n->left, n, val, ok, autoBal);
            
            if (n->left) 
                n->left->parent = n;
        }
        else
        {
            n->right = insertRec(n->right, n, val, ok, autoBal);
            
            if (n->right) 
                n->right->parent = n;
        }

        if (!ok) 
            return n;
        update(n);
        
        return autoBal ? rebalance(n) : n;
    }

    bool insert(int val, bool autoBal = true)
    {
        bool ok = false;
        root = insertRec(root, nullptr, val, ok, autoBal);
        
        if (root) 
            root->parent = nullptr;
        
        return ok;
    }

    ScratchAVLNode* findMin(ScratchAVLNode* n)
    {
        ScratchAVLNode* c = n;
        
        while (c && c->left) 
            c = c->left;
        
        return c;
    }

    ScratchAVLNode* deleteRec(ScratchAVLNode* n, int val, bool& ok, bool autoBal)
    {
        if (!n) 
        { 
            ok = false; 
            return nullptr; 
        }
        
        if (val < n->value)
        {
            n->left = deleteRec(n->left, val, ok, autoBal);
            if (n->left) n->left->parent = n;
        }
        else if (val > n->value)
        {
            n->right = deleteRec(n->right, val, ok, autoBal);
            if (n->right) n->right->parent = n;
        }
        else
        {
            ok = true;
            if (!n->left || !n->right)
            {
                ScratchAVLNode* t = n->left ? n->left : n->right;
                if (!t) { delete n; return nullptr; }
                t->parent = n->parent;
                delete n;
                n = t;
            }
            else
            {
                ScratchAVLNode* succ = findMin(n->right);
                n->value = succ->value;
                n->right = deleteRec(n->right, succ->value, ok, autoBal);
                if (n->right) n->right->parent = n;
            }
        }
        if (!n) 
            return nullptr;
        
        update(n);
        
        return autoBal ? rebalance(n) : n;
    }

    bool deleteKey(int val, bool autoBal = true)
    {
        bool ok = false;
        root = deleteRec(root, val, ok, autoBal);
        if (root) root->parent = nullptr;
        return ok;
    }

    ScratchAVLNode* search(int val)
    {
        ScratchAVLNode* c = root;
        while (c)
        {
            if (c->value == val) return c;
            if (val < c->value) c = c->left;
            else c = c->right;
        }
        return nullptr;
    }

    bool rotateLL(int val = -1)
    {
        ScratchAVLNode* target = (val != -1) ? search(val) : root;
        
        if (!target || !target->left) 
            return false;
        
        ScratchAVLNode* p = target->parent;
        ScratchAVLNode* newSub = rotRight(target);
        
        if (!p) 
            root = newSub;
        else if (p->left == target) 
            p->left = newSub;
        else 
            p->right = newSub;
        
        newSub->parent = p;
        return true;
    }

    bool rotateRR(int val = -1)
    {
        ScratchAVLNode* target = (val != -1) ? search(val) : root;
        
        if (!target || !target->right) 
            return false;
        
        ScratchAVLNode* p = target->parent;
        ScratchAVLNode* newSub = rotLeft(target);
        
        if (!p) 
            root = newSub;
        else if (p->left == target) 
            p->left = newSub;
        else 
            p->right = newSub;
        
        newSub->parent = p;
        return true;
    }

    bool rotateLR(int val = -1)
    {
        ScratchAVLNode* target = (val != -1) ? search(val) : root;
        
        if (!target || !target->left || !target->left->right) 
            return false;
        
        target->left = rotLeft(target->left);
        target->left->parent = target;
        
        ScratchAVLNode* p = target->parent;
        ScratchAVLNode* newSub = rotRight(target);
        
        if (!p) 
            root = newSub;
        else if (p->left == target) 
            p->left = newSub;
        else 
            p->right = newSub;
        
        newSub->parent = p;
        return true;
    }

    bool rotateRL(int val = -1)
    {
        ScratchAVLNode* target = (val != -1) ? search(val) : root;
        
        if (!target || !target->right || !target->right->left) 
            return false;
        
        target->right = rotRight(target->right);
        target->right->parent = target;
        ScratchAVLNode* p = target->parent;
        ScratchAVLNode* newSub = rotLeft(target);
        
        if (!p) 
            root = newSub;
        else if (p->left == target) 
            p->left = newSub;
        else 
            p->right = newSub;
        
        newSub->parent = p;
        return true;
    }

    bool checkBal(ScratchAVLNode* n) const
    {
        if (!n) 
            return true;
        
        int bf = getBF(n);
        
        if (bf < -1 || bf > 1) 
            return false;
        
        return checkBal(n->left) && checkBal(n->right);
    }

    bool isBalanced() const { return checkBal(root); }
    int totalNodes() const { return getSize(root); }
    int treeHeight() const { return getHeight(root); }

    TreeNode* convertToTN(ScratchAVLNode* n, TreeNode* p) const
    {
        if (!n) 
            return nullptr;
        
        TreeNode* tn = new TreeNode(n->id, n->value, p);
        
        tn->left = convertToTN(n->left, tn);
        tn->right = convertToTN(n->right, tn);
        
        return tn;
    }

    TreeNode* toBinaryTree() const { return convertToTN(root, nullptr); }

    static void freeTN(TreeNode* n)
    {
        if (!n) 
            return;
        
        freeTN(n->left);
        freeTN(n->right);
        
        delete n;
    }
};

static bool rotateBinaryTreeLL(BinaryTree& tree, int selectedVal)
{
    TreeNode* root = tree.getRoot();
    TreeNode* n = (selectedVal != -1) ? tree.findNode(selectedVal) : root;
    
    if (!n || !n->left) 
        return false;
    
    TreeNode* x = n->left;
    TreeNode* T2 = x->right;
    TreeNode* p = n->parent;
    
    x->right = n;
    n->parent = x;
    n->left = T2;
    
    if (T2) 
        T2->parent = n;
   
    x->parent = p;
    
    if (!p) 
        tree.setRoot(x);
    else if (p->left == n) 
        p->left = x;
    else 
        p->right = x;
    
    return true;
}

static bool rotateBinaryTreeRR(BinaryTree& tree, int selectedVal)
{
    TreeNode* root = tree.getRoot();
    TreeNode* n = (selectedVal != -1) ? tree.findNode(selectedVal) : root;
    
    if (!n || !n->right) 
        return false;
    
    TreeNode* y = n->right;
    TreeNode* T2 = y->left;
    TreeNode* p = n->parent;
    
    y->left = n;
    n->parent = y;
    n->right = T2;
    
    if (T2) 
        T2->parent = n;
    y->parent = p;
   
    if (!p) 
        tree.setRoot(y);
    else if (p->left == n) 
        p->left = y;
    else 
        p->right = y;
    return true;
}

class ScratchHeap
{
public:
    int data[32];
    int size;

    ScratchHeap() : size(0)
    {
        for (int i = 0; i < 32; ++i) 
            data[i] = 0;
    }

    void siftUp(int idx)
    {
        int c = idx;
        while (c > 0)
        {
            int p = (c - 1) / 2;
            
            if (data[c] > data[p])
            {
                int t = data[c]; 
                data[c] = data[p]; 
                data[p] = t;
                c = p;
            }
            else 
                break;
        }
    }

    void siftDown(int idx, int count)
    {
        int c = idx;
        
        while (true)
        {
            int largest = c;
            int l = 2 * c + 1;
            int r = 2 * c + 2;
        
            if (l < count && data[l] > data[largest]) 
                largest = l;
            
            if (r < count && data[r] > data[largest]) 
                largest = r;
            
            if (largest != c)
            {
                int t = data[c]; data[c] = data[largest]; data[largest] = t;
                c = largest;
            }
            else 
                break;
        }
    }

    bool insert(int val)
    {
        if (size >= 32) 
            return false;
        int idx = size++;
        data[idx] = val;
        siftUp(idx);
        return true;
    }

    int extractMax()
    {
        if (size <= 0) 
            return -1;
       
        int maxV = data[0];
        data[0] = data[size - 1];
        size--;
        
        if (size > 0) 
            siftDown(0, size);
        return maxV;
    }

    void build(const int arr[], int n)
    {
        size = minInt(32, n);
        
        for (int i = 0; i < size; ++i) 
            data[i] = arr[i];
        
        for (int i = (size - 2) / 2; i >= 0; --i) 
            siftDown(i, size);
    }

    void sort()
    {
        int origSize = size;
        
        for (int i = (size - 2) / 2; i >= 0; --i) 
            siftDown(i, size);
        
        for (int i = size - 1; i > 0; --i)
        {
            int t = data[0]; data[0] = data[i]; data[i] = t;
            siftDown(0, i);
        }
        
        size = origSize;
    }

    bool isHeap() const
    {
        for (int i = 0; i <= (size - 2) / 2; ++i)
        {
            int l = 2 * i + 1;
            int r = 2 * i + 2;
            if (l < size && data[i] < data[l]) return false;
            if (r < size && data[i] < data[r]) return false;
        }
        return true;
    }
};




struct SimpleLog
{
    char messages[16][128];
    int count = 0;

    void record(const string& msg)
    {
        if (count < 16)
        {
            safeCopyStr(messages[count++], msg, 128);
        }
        else
        {
            for (int i = 0; i < 15; ++i)
                safeCopyStr(messages[i], messages[i + 1], 128);
            safeCopyStr(messages[15], msg, 128);
        }
    }

    void clear() { count = 0; }
};




static void drawEventLog(RenderWindow& window, const Font& font, float sx, float sy, const SimpleLog& logs, Color textColor)
{
    
    int showCount = minInt(6, logs.count);
    for (int i = 0; i < showCount; ++i)
    {
        const char* raw = logs.messages[logs.count - showCount + i];
        char lineBuf[32];
        int len = 0;
        while (raw[len] && len < 26) { lineBuf[len] = raw[len]; len++; }
        if (raw[len] && len == 26)
        {
            lineBuf[23] = '.';
            lineBuf[24] = '.';
            lineBuf[25] = '.';
        }
        lineBuf[len] = '\0';

        Text lt(font, lineBuf, static_cast<unsigned int>(14 * sy));
        lt.setFillColor(textColor);
        lt.setPosition({ 1575.f * sx, (305.f + static_cast<float>(i) * 29.f) * sy });
        window.draw(lt);
    }
}

static void drawSpeedSlider(RenderWindow& window, const Font& font, float sx, float sy, float speedMultiplier, float speedRatio, Color themeColor, Color fillColor)
{
    (void)font;
    (void)speedMultiplier;

    
    float trackX = 976.f;
    float trackY = 983.f;
    float trackW = 529.f;
    float trackH = 31.f;

    if (speedRatio > 0.01f)
    {
        RectangleShape fill({ trackW * speedRatio * sx, (trackH - 6.f) * sy });
        fill.setPosition({ trackX * sx, (trackY + 3.f) * sy });
        fill.setFillColor(fillColor);
        window.draw(fill);
    }

    
    float knobX = (trackX + speedRatio * trackW) * sx;
    float knobY = (trackY + trackH * 0.5f) * sy;
    float knobRadius = 15.f * sy;

    CircleShape knob(knobRadius);
    knob.setOrigin({ knobRadius, knobRadius });
    knob.setPosition({ knobX, knobY });
    knob.setFillColor(Color::White);
    knob.setOutlineThickness(3.f * sx);
    knob.setOutlineColor(themeColor);
    window.draw(knob);
}




static string runSandboxBinary(RenderWindow& window, const Font& font)
{
    Texture bgTex;
    string path = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_Binary.png", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_Binary.png");
    if (!bgTex.loadFromFile(path)) return "MENU";

    Sprite bg(bgTex);
    BinaryTree tree;
    int pre[7] = { 50, 30, 20, 40, 70, 60, 80 };
    int inArr[7]  = { 20, 30, 40, 50, 60, 70, 80 };
    tree.build(pre, inArr, 7);

    SimpleLog logs;
    logs.record("[SANDBOX] Binary Tree initialized with 7 nodes");

    int operations = 0;
    string inputKey = "25";
    string rangeLo = "20", rangeHi = "65";
    int activeInput = -1;
    int selectedVal = -1;
    int selectedRot = 0;
    GameNodePos positions[64];
    int posCount = 0;

    float speedRatio = 0.5f;
    float speedMultiplier = 1.0f;
    bool draggingSpeed = false;

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>()) { window.close(); return "EXIT GAME"; }
            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape) return "SANDBOX_MAP";
                if (kp->code == Keyboard::Key::Backspace)
                {
                    if (activeInput == 0 && !inputKey.empty()) inputKey.pop_back();
                    if (activeInput == 1 && !rangeLo.empty()) rangeLo.pop_back();
                    if (activeInput == 2 && !rangeHi.empty()) rangeHi.pop_back();
                }
            }
            if (const auto* txt = event->getIf<Event::TextEntered>())
            {
                if (txt->unicode >= '0' && txt->unicode <= '9')
                {
                    char c = static_cast<char>(txt->unicode);
                    if (activeInput == 0 && inputKey.size() < 3) inputKey += c;
                    if (activeInput == 1 && rangeLo.size() < 3) rangeLo += c;
                    if (activeInput == 2 && rangeHi.size() < 3) rangeHi += c;
                }
            }
            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    Vector2u winSize = window.getSize();
                    Vector2u imgSize = bgTex.getSize();
                    float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
                    float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
                    Vector2i mpos = Mouse::getPosition(window);
                    float mx = static_cast<float>(mpos.x) / sx;
                    float my = static_cast<float>(mpos.y) / sy;

                    if (isInside(mx, my, 25.f, 20.f, 80.f, 70.f)) return "SANDBOX_MAP";

                    
                    if (isInside(mx, my, 965.f, 965.f, 550.f, 65.f))
                    {
                        draggingSpeed = true;
                        speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
                        speedMultiplier = 0.5f + speedRatio * 2.5f;
                    }

                    if (isInside(mx, my, 1718.f, 528.f, 108.f, 72.f)) activeInput = 0;
                    else if (isInside(mx, my, 1563.f, 515.f, 150.f, 99.f))
                    {
                        if (!inputKey.empty())
                        {
                            int val = stoi(inputKey);
                            TraversalResult tr = tree.levelOrder();
                            TreeNode* attach = nullptr;
                            for (int i = 0; i < tr.size; ++i)
                            {
                                TreeNode* n = tree.findNode(tr.get(i));
                                if (n && (!n->left || !n->right)) { attach = n; break; }
                            }
                            if (attach)
                            {
                                if (!attach->left) attach->left = new TreeNode(val, val, attach);
                                else attach->right = new TreeNode(val, val, attach);
                                logs.record("[INSERT] Attached " + to_string(val) + " to node " + to_string(attach->value));
                                operations++;
                            }
                        }
                    }
                    else if (isInside(mx, my, 1563.f, 625.f, 299.f, 99.f))
                    {
                        if (selectedVal != -1)
                        {
                            TreeNode* n = tree.findNode(selectedVal);
                            if (n && n->parent)
                            {
                                if (n->parent->left == n) n->parent->left = nullptr;
                                else n->parent->right = nullptr;
                                logs.record("[DELETE] Removed node " + to_string(n->value));
                                delete n;
                                selectedVal = -1;
                                operations++;
                            }
                        }
                    }
                    
                    for (int r = 0; r < 2; ++r)
                    {
                        float rY = (r == 0) ? 782.f : 822.f;
                        if (isInside(mx, my, 1715.f, rY - 16.f, 140.f, 32.f))
                        {
                            selectedRot = r;
                            if (selectedRot == 0)
                            {
                                if (rotateBinaryTreeLL(tree, selectedVal))
                                {
                                    logs.record("[ROTATE] LL (Right Rotate)");
                                    operations++;
                                }
                                else logs.record("[INFO] LL not applicable");
                            }
                            else
                            {
                                if (rotateBinaryTreeRR(tree, selectedVal))
                                {
                                    logs.record("[ROTATE] RR (Left Rotate)");
                                    operations++;
                                }
                                else logs.record("[INFO] RR not applicable");
                            }
                            break;
                        }
                    }
                    if (isInside(mx, my, 1566.f, 739.f, 140.f, 148.f))
                    {
                        if (selectedRot == 0)
                        {
                            if (rotateBinaryTreeLL(tree, selectedVal))
                            {
                                logs.record("[ROTATE] LL (Right Rotate)");
                                operations++;
                            }
                            else logs.record("[INFO] LL not applicable");
                        }
                        else
                        {
                            if (rotateBinaryTreeRR(tree, selectedVal))
                            {
                                logs.record("[ROTATE] RR (Left Rotate)");
                                operations++;
                            }
                            else logs.record("[INFO] RR not applicable");
                        }
                    }
                    else if (isInside(mx, my, 1586.f, 977.f, 97.f, 67.f)) activeInput = 1;
                    else if (isInside(mx, my, 1750.f, 977.f, 97.f, 67.f)) activeInput = 2;
                    else if (isInside(mx, my, 37.f, 945.f, 311.f, 117.f))
                    {
                        tree.clear();
                        tree.build(pre, inArr, 7);
                        logs.record("[RESET] Reset Binary Tree");
                        operations = 0;
                        selectedVal = -1;
                    }

                    for (int i = 0; i < posCount; ++i)
                    {
                        float dx = mx - positions[i].x;
                        float dy = my - positions[i].y;
                        if (dx * dx + dy * dy <= positions[i].radius * positions[i].radius)
                        {
                            selectedVal = positions[i].value;
                            logs.record("[SELECT] Node " + to_string(selectedVal));
                            break;
                        }
                    }
                }
            }
            if (const auto* mbr = event->getIf<Event::MouseButtonReleased>())
            {
                if (mbr->button == Mouse::Button::Left)
                {
                    draggingSpeed = false;
                }
            }
        }

        Vector2u winSize = window.getSize();
        Vector2u imgSize = bgTex.getSize();
        if (winSize.x == 0 || winSize.y == 0) continue;

        float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
        float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
        bg.setScale({ sx, sy });

        Vector2i mpos = Mouse::getPosition(window);
        float mx = static_cast<float>(mpos.x) / sx;
        float my = static_cast<float>(mpos.y) / sy;
        if (draggingSpeed)
        {
            speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
            speedMultiplier = 0.5f + speedRatio * 2.5f;
        }

        posCount = layoutGeneralTree(tree.getRoot(), 34.f, 250.f, 1500.f, 660.f, positions, 64);

        int rLo = rangeLo.empty() ? 0 : stoi(rangeLo);
        int rHi = rangeHi.empty() ? 999 : stoi(rangeHi);
        for (int i = 0; i < posCount; ++i)
        {
            positions[i].isHighlighted = (positions[i].value >= rLo && positions[i].value <= rHi);
            positions[i].isGreyed = !positions[i].isHighlighted;
            positions[i].isSelected = (positions[i].value == selectedVal);
        }

        window.clear();
        window.draw(bg);

        
        Text hText(font, to_string(tree.height()), static_cast<unsigned int>(34 * sy));
        hText.setFillColor(Color::White);
        drawCenteredText(window, hText, 175.f * sx, 188.f * sy);

        Text opText(font, to_string(operations), static_cast<unsigned int>(34 * sy));
        opText.setFillColor(Color(78, 255, 152));
        drawCenteredText(window, opText, 496.5f * sx, 188.f * sy);

        Text nodeCntText(font, to_string(tree.nodeCount()), static_cast<unsigned int>(34 * sy));
        nodeCntText.setFillColor(Color(78, 126, 251));
        drawCenteredText(window, nodeCntText, 818.f * sx, 188.f * sy);

        Text banner(font, "BINARY TREE SANDBOX | Free-Form Tree Operations", static_cast<unsigned int>(28 * sy));
        banner.setFillColor(Color(200, 220, 240));
        drawCenteredText(window, banner, 1431.5f * sx, 158.f * sy);

        
        for (int i = 0; i < posCount; ++i)
        {
            if (positions[i].parentId != -1)
            {
                for (int j = 0; j < posCount; ++j)
                {
                    if (positions[j].id == positions[i].parentId)
                    {
                        drawThickLine(window, { positions[i].x * sx, positions[i].y * sy }, { positions[j].x * sx, positions[j].y * sy }, 2.5f * sx, Color(140, 160, 190, 180));
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < posCount; ++i)
        {
            CircleShape c(positions[i].radius * sx);
            c.setOrigin({ positions[i].radius * sx, positions[i].radius * sx });
            c.setPosition({ positions[i].x * sx, positions[i].y * sy });

            if (positions[i].isSelected)
            {
                c.setFillColor(Color(255, 215, 0, 220));
                c.setOutlineThickness(3.f * sx);
                c.setOutlineColor(Color::White);
            }
            else if (positions[i].isHighlighted)
            {
                c.setFillColor(Color(40, 120, 90, 220));
                c.setOutlineThickness(2.f * sx);
                c.setOutlineColor(Color(78, 255, 152));
            }
            else
            {
                c.setFillColor(Color(40, 45, 55, 180));
                c.setOutlineThickness(1.5f * sx);
                c.setOutlineColor(Color(80, 90, 100));
            }
            window.draw(c);

            Text vt(font, to_string(positions[i].value), static_cast<unsigned int>(20 * sx));
            vt.setFillColor(Color::White);
            drawCenteredText(window, vt, positions[i].x * sx, positions[i].y * sy);
        }

        
        drawEventLog(window, font, sx, sy, logs, Color(200, 220, 240));

        
        const float rotY[2] = { 782.f, 822.f };
        CircleShape rDot(5.5f * sx);
        rDot.setOrigin({ 5.5f * sx, 5.5f * sx });
        rDot.setPosition({ 1733.f * sx, rotY[selectedRot] * sy });
        rDot.setFillColor(Color(78, 255, 152));
        window.draw(rDot);

        
        Text inT(font, inputKey + (activeInput == 0 ? "_" : ""), static_cast<unsigned int>(24 * sy));
        inT.setFillColor(Color(78, 255, 152));
        drawCenteredText(window, inT, 1772.f * sx, 564.f * sy);

        
        Text rLoT(font, rangeLo + (activeInput == 1 ? "_" : ""), static_cast<unsigned int>(20 * sy));
        drawCenteredText(window, rLoT, 1634.5f * sx, 1010.5f * sy);

        Text rHiT(font, rangeHi + (activeInput == 2 ? "_" : ""), static_cast<unsigned int>(20 * sy));
        drawCenteredText(window, rHiT, 1798.5f * sx, 1010.5f * sy);

        
        drawSpeedSlider(window, font, sx, sy, speedMultiplier, speedRatio, Color(78, 255, 152), Color(78, 255, 152, 180));

        window.display();
    }

    return "SANDBOX_MAP";
}

static string runSandboxBST(RenderWindow& window, const Font& font)
{
    Texture bgTex;
    string path = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_BST.png", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_BST.png");
    if (!bgTex.loadFromFile(path)) return "MENU";

    Sprite bg(bgTex);
    ScratchAVL bst;
    int keys[7] = { 50, 30, 70, 20, 40, 60, 80 };
    for (int k : keys) bst.insert(k, false);

    SimpleLog logs;
    logs.record("[SANDBOX BST] Binary Search Tree initialized");

    int operations = 0;
    string inputKey = "45";
    string rangeLo = "25", rangeHi = "75";
    int activeInput = -1;
    int selectedVal = -1;
    int selectedRot = 0;
    GameNodePos positions[64];
    int posCount = 0;

    float speedRatio = 0.5f;
    float speedMultiplier = 1.0f;
    bool draggingSpeed = false;

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>()) { window.close(); return "EXIT GAME"; }
            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape) return "SANDBOX_MAP";
                if (kp->code == Keyboard::Key::Backspace)
                {
                    if (activeInput == 0 && !inputKey.empty()) inputKey.pop_back();
                    if (activeInput == 1 && !rangeLo.empty()) rangeLo.pop_back();
                    if (activeInput == 2 && !rangeHi.empty()) rangeHi.pop_back();
                }
            }
            if (const auto* txt = event->getIf<Event::TextEntered>())
            {
                if (txt->unicode >= '0' && txt->unicode <= '9')
                {
                    char c = static_cast<char>(txt->unicode);
                    if (activeInput == 0 && inputKey.size() < 3) inputKey += c;
                    if (activeInput == 1 && rangeLo.size() < 3) rangeLo += c;
                    if (activeInput == 2 && rangeHi.size() < 3) rangeHi += c;
                }
            }
            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    Vector2u winSize = window.getSize();
                    Vector2u imgSize = bgTex.getSize();
                    float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
                    float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
                    Vector2i mpos = Mouse::getPosition(window);
                    float mx = static_cast<float>(mpos.x) / sx;
                    float my = static_cast<float>(mpos.y) / sy;

                    if (isInside(mx, my, 25.f, 20.f, 80.f, 70.f)) return "SANDBOX_MAP";

                    
                    if (isInside(mx, my, 965.f, 965.f, 550.f, 65.f))
                    {
                        draggingSpeed = true;
                        speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
                        speedMultiplier = 0.5f + speedRatio * 2.5f;
                    }

                    if (isInside(mx, my, 1718.f, 528.f, 108.f, 72.f)) activeInput = 0;
                    else if (isInside(mx, my, 1563.f, 515.f, 150.f, 99.f))
                    {
                        if (!inputKey.empty())
                        {
                            int val = stoi(inputKey);
                            if (bst.insert(val, false))
                            {
                                logs.record("[INSERT] Added " + to_string(val) + " to BST");
                                operations++;
                            }
                            else logs.record("[REJECT] Key " + to_string(val) + " duplicate");
                        }
                    }
                    else if (isInside(mx, my, 1563.f, 625.f, 299.f, 99.f))
                    {
                        int target = (selectedVal != -1) ? selectedVal : (!inputKey.empty() ? stoi(inputKey) : -1);
                        if (target != -1)
                        {
                            if (bst.deleteKey(target, false))
                            {
                                logs.record("[DELETE] Removed key " + to_string(target));
                                selectedVal = -1;
                                operations++;
                            }
                        }
                    }
                    
                    for (int r = 0; r < 2; ++r)
                    {
                        float rY = (r == 0) ? 782.f : 822.f;
                        if (isInside(mx, my, 1715.f, rY - 16.f, 140.f, 32.f))
                        {
                            selectedRot = r;
                            if (selectedRot == 0)
                            {
                                if (bst.rotateLL(selectedVal))
                                {
                                    logs.record("[ROTATE] LL (Right Rotate)");
                                    operations++;
                                }
                                else logs.record("[INFO] LL not applicable");
                            }
                            else
                            {
                                if (bst.rotateRR(selectedVal))
                                {
                                    logs.record("[ROTATE] RR (Left Rotate)");
                                    operations++;
                                }
                                else logs.record("[INFO] RR not applicable");
                            }
                            break;
                        }
                    }
                    if (isInside(mx, my, 1566.f, 739.f, 140.f, 148.f))
                    {
                        if (selectedRot == 0)
                        {
                            if (bst.rotateLL(selectedVal))
                            {
                                logs.record("[ROTATE] LL (Right Rotate)");
                                operations++;
                            }
                            else logs.record("[INFO] LL not applicable");
                        }
                        else
                        {
                            if (bst.rotateRR(selectedVal))
                            {
                                logs.record("[ROTATE] RR (Left Rotate)");
                                operations++;
                            }
                            else logs.record("[INFO] RR not applicable");
                        }
                    }
                    else if (isInside(mx, my, 1586.f, 977.f, 97.f, 67.f)) activeInput = 1;
                    else if (isInside(mx, my, 1750.f, 977.f, 97.f, 67.f)) activeInput = 2;
                    else if (isInside(mx, my, 37.f, 945.f, 311.f, 117.f))
                    {
                        bst.clear();
                        for (int k : keys) bst.insert(k, false);
                        logs.record("[RESET] Reset BST");
                        operations = 0;
                        selectedVal = -1;
                    }

                    for (int i = 0; i < posCount; ++i)
                    {
                        float dx = mx - positions[i].x;
                        float dy = my - positions[i].y;
                        if (dx * dx + dy * dy <= positions[i].radius * positions[i].radius)
                        {
                            selectedVal = positions[i].value;
                            logs.record("[SELECT] Node " + to_string(selectedVal));
                            break;
                        }
                    }
                }
            }
            if (const auto* mbr = event->getIf<Event::MouseButtonReleased>())
            {
                if (mbr->button == Mouse::Button::Left)
                {
                    draggingSpeed = false;
                }
            }
        }

        Vector2u winSize = window.getSize();
        Vector2u imgSize = bgTex.getSize();
        if (winSize.x == 0 || winSize.y == 0) continue;

        float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
        float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
        bg.setScale({ sx, sy });

        Vector2i mpos = Mouse::getPosition(window);
        float mx = static_cast<float>(mpos.x) / sx;
        float my = static_cast<float>(mpos.y) / sy;
        if (draggingSpeed)
        {
            speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
            speedMultiplier = 0.5f + speedRatio * 2.5f;
        }

        TreeNode* tmp = bst.toBinaryTree();
        posCount = layoutGeneralTree(tmp, 34.f, 250.f, 1500.f, 660.f, positions, 64);
        ScratchAVL::freeTN(tmp);

        int rLo = rangeLo.empty() ? 0 : stoi(rangeLo);
        int rHi = rangeHi.empty() ? 999 : stoi(rangeHi);
        for (int i = 0; i < posCount; ++i)
        {
            positions[i].isHighlighted = (positions[i].value >= rLo && positions[i].value <= rHi);
            positions[i].isGreyed = !positions[i].isHighlighted;
            positions[i].isSelected = (positions[i].value == selectedVal);
        }

        window.clear();
        window.draw(bg);

        
        Text hText(font, to_string(bst.treeHeight()), static_cast<unsigned int>(34 * sy));
        hText.setFillColor(Color::White);
        drawCenteredText(window, hText, 175.f * sx, 188.f * sy);

        Text opText(font, to_string(operations), static_cast<unsigned int>(34 * sy));
        opText.setFillColor(Color(255, 120, 120));
        drawCenteredText(window, opText, 496.5f * sx, 188.f * sy);

        Text stat(font, "BST ORDERED", static_cast<unsigned int>(26 * sy));
        stat.setFillColor(Color(78, 255, 152));
        drawCenteredText(window, stat, 818.f * sx, 188.f * sy);

        Text banner(font, "BST SANDBOX | Search, Insert & Successor Deletion", static_cast<unsigned int>(28 * sy));
        banner.setFillColor(Color(240, 200, 200));
        drawCenteredText(window, banner, 1431.5f * sx, 158.f * sy);

        
        for (int i = 0; i < posCount; ++i)
        {
            if (positions[i].parentId != -1)
            {
                for (int j = 0; j < posCount; ++j)
                {
                    if (positions[j].id == positions[i].parentId)
                    {
                        drawThickLine(window, { positions[i].x * sx, positions[i].y * sy }, { positions[j].x * sx, positions[j].y * sy }, 2.5f * sx, Color(160, 100, 100, 180));
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < posCount; ++i)
        {
            CircleShape c(positions[i].radius * sx);
            c.setOrigin({ positions[i].radius * sx, positions[i].radius * sx });
            c.setPosition({ positions[i].x * sx, positions[i].y * sy });

            if (positions[i].isSelected)
            {
                c.setFillColor(Color(255, 215, 0, 220));
                c.setOutlineThickness(3.f * sx);
                c.setOutlineColor(Color::White);
            }
            else if (positions[i].isHighlighted)
            {
                c.setFillColor(Color(120, 40, 50, 220));
                c.setOutlineThickness(2.f * sx);
                c.setOutlineColor(Color(255, 100, 100));
            }
            else
            {
                c.setFillColor(Color(45, 35, 40, 180));
                c.setOutlineThickness(1.5f * sx);
                c.setOutlineColor(Color(90, 70, 75));
            }
            window.draw(c);

            Text vt(font, to_string(positions[i].value), static_cast<unsigned int>(20 * sx));
            vt.setFillColor(Color::White);
            drawCenteredText(window, vt, positions[i].x * sx, positions[i].y * sy);
        }

        
        drawEventLog(window, font, sx, sy, logs, Color(240, 210, 210));

        
        const float rotY[2] = { 782.f, 822.f };
        CircleShape rDot(5.5f * sx);
        rDot.setOrigin({ 5.5f * sx, 5.5f * sx });
        rDot.setPosition({ 1733.f * sx, rotY[selectedRot] * sy });
        rDot.setFillColor(Color(255, 140, 140));
        window.draw(rDot);

        
        Text inT(font, inputKey + (activeInput == 0 ? "_" : ""), static_cast<unsigned int>(24 * sy));
        inT.setFillColor(Color(255, 140, 140));
        drawCenteredText(window, inT, 1772.f * sx, 564.f * sy);

        
        Text rLoT(font, rangeLo + (activeInput == 1 ? "_" : ""), static_cast<unsigned int>(20 * sy));
        drawCenteredText(window, rLoT, 1634.5f * sx, 1010.5f * sy);

        Text rHiT(font, rangeHi + (activeInput == 2 ? "_" : ""), static_cast<unsigned int>(20 * sy));
        drawCenteredText(window, rHiT, 1798.5f * sx, 1010.5f * sy);

        
        drawSpeedSlider(window, font, sx, sy, speedMultiplier, speedRatio, Color(255, 120, 120), Color(255, 120, 120, 180));

        window.display();
    }

    return "SANDBOX_MAP";
}

static string runSandboxAVL(RenderWindow& window, const Font& font)
{
    Texture bgTex;
    string path = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_AVL.png", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_AVL.png");
    if (!bgTex.loadFromFile(path)) return "MENU";

    Sprite bg(bgTex);
    ScratchAVL avl;
    int keys[7] = { 40, 20, 60, 10, 30, 50, 70 };
    for (int k : keys) avl.insert(k, true);

    SimpleLog logs;
    logs.record("[SANDBOX AVL] Self-Balancing AVL Tree initialized");

    int operations = 0;
    string inputKey = "25";
    string rangeLo = "15", rangeHi = "65";
    int activeInput = -1;
    int selectedVal = -1;
    int selectedRot = 0;
    GameNodePos positions[64];
    int posCount = 0;

    float speedRatio = 0.5f;
    float speedMultiplier = 1.0f;
    bool draggingSpeed = false;

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>()) { window.close(); return "EXIT GAME"; }
            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape) return "SANDBOX_MAP";
                if (kp->code == Keyboard::Key::Backspace)
                {
                    if (activeInput == 0 && !inputKey.empty()) inputKey.pop_back();
                    if (activeInput == 1 && !rangeLo.empty()) rangeLo.pop_back();
                    if (activeInput == 2 && !rangeHi.empty()) rangeHi.pop_back();
                }
            }
            if (const auto* txt = event->getIf<Event::TextEntered>())
            {
                if (txt->unicode >= '0' && txt->unicode <= '9')
                {
                    char c = static_cast<char>(txt->unicode);
                    if (activeInput == 0 && inputKey.size() < 3) inputKey += c;
                    if (activeInput == 1 && rangeLo.size() < 3) rangeLo += c;
                    if (activeInput == 2 && rangeHi.size() < 3) rangeHi += c;
                }
            }
            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    Vector2u winSize = window.getSize();
                    Vector2u imgSize = bgTex.getSize();
                    float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
                    float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
                    Vector2i mpos = Mouse::getPosition(window);
                    float mx = static_cast<float>(mpos.x) / sx;
                    float my = static_cast<float>(mpos.y) / sy;

                    if (isInside(mx, my, 25.f, 20.f, 80.f, 70.f)) return "SANDBOX_MAP";

                    
                    if (isInside(mx, my, 965.f, 965.f, 550.f, 65.f))
                    {
                        draggingSpeed = true;
                        speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
                        speedMultiplier = 0.5f + speedRatio * 2.5f;
                    }

                    if (isInside(mx, my, 1718.f, 528.f, 108.f, 72.f)) activeInput = 0;
                    else if (isInside(mx, my, 1563.f, 515.f, 150.f, 99.f))
                    {
                        if (!inputKey.empty())
                        {
                            int val = stoi(inputKey);
                            if (avl.insert(val, true))
                            {
                                logs.record("[AVL INSERT] Auto-balanced " + to_string(val));
                                operations++;
                            }
                        }
                    }
                    else if (isInside(mx, my, 1563.f, 625.f, 299.f, 99.f))
                    {
                        int target = (selectedVal != -1) ? selectedVal : (!inputKey.empty() ? stoi(inputKey) : -1);
                        if (target != -1)
                        {
                            if (avl.deleteKey(target, true))
                            {
                                logs.record("[AVL DELETE] Auto-balanced " + to_string(target));
                                selectedVal = -1;
                                operations++;
                            }
                        }
                    }
                    
                    for (int r = 0; r < 4; ++r)
                    {
                        float rY = (r == 0) ? 771.f : ((r == 1) ? 801.f : ((r == 2) ? 833.f : 864.f));
                        if (isInside(mx, my, 1715.f, rY - 14.f, 140.f, 28.f))
                        {
                            selectedRot = r;
                            bool ok = false;
                            if (selectedRot == 0) ok = avl.rotateLL(selectedVal);
                            else if (selectedRot == 1) ok = avl.rotateRR(selectedVal);
                            else if (selectedRot == 2) ok = avl.rotateLR(selectedVal);
                            else if (selectedRot == 3) ok = avl.rotateRL(selectedVal);

                            const char* rotNames[4] = { "LL", "RR", "LR", "RL" };
                            if (ok)
                            {
                                logs.record(string("[ROTATE] AVL ") + rotNames[selectedRot] + " executed");
                                operations++;
                            }
                            else logs.record(string("[INFO] ") + rotNames[selectedRot] + " not applicable");
                            break;
                        }
                    }
                    if (isInside(mx, my, 1566.f, 739.f, 140.f, 148.f))
                    {
                        bool ok = false;
                        if (selectedRot == 0) ok = avl.rotateLL(selectedVal);
                        else if (selectedRot == 1) ok = avl.rotateRR(selectedVal);
                        else if (selectedRot == 2) ok = avl.rotateLR(selectedVal);
                        else if (selectedRot == 3) ok = avl.rotateRL(selectedVal);

                        const char* rotNames[4] = { "LL", "RR", "LR", "RL" };
                        if (ok)
                        {
                            logs.record(string("[ROTATE] AVL ") + rotNames[selectedRot] + " executed");
                            operations++;
                        }
                        else logs.record(string("[INFO] ") + rotNames[selectedRot] + " not applicable");
                    }
                    else if (isInside(mx, my, 1586.f, 977.f, 97.f, 67.f)) activeInput = 1;
                    else if (isInside(mx, my, 1750.f, 977.f, 97.f, 67.f)) activeInput = 2;
                    else if (isInside(mx, my, 37.f, 945.f, 311.f, 117.f))
                    {
                        avl.clear();
                        for (int k : keys) avl.insert(k, true);
                        logs.record("[RESET] Reset AVL Tree");
                        operations = 0;
                        selectedVal = -1;
                    }

                    for (int i = 0; i < posCount; ++i)
                    {
                        float dx = mx - positions[i].x;
                        float dy = my - positions[i].y;
                        if (dx * dx + dy * dy <= positions[i].radius * positions[i].radius)
                        {
                            selectedVal = positions[i].value;
                            logs.record("[SELECT] Node " + to_string(selectedVal));
                            break;
                        }
                    }
                }
            }
            if (const auto* mbr = event->getIf<Event::MouseButtonReleased>())
            {
                if (mbr->button == Mouse::Button::Left)
                {
                    draggingSpeed = false;
                }
            }
        }

        Vector2u winSize = window.getSize();
        Vector2u imgSize = bgTex.getSize();
        if (winSize.x == 0 || winSize.y == 0) continue;

        float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
        float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
        bg.setScale({ sx, sy });

        Vector2i mpos = Mouse::getPosition(window);
        float mx = static_cast<float>(mpos.x) / sx;
        float my = static_cast<float>(mpos.y) / sy;
        if (draggingSpeed)
        {
            speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
            speedMultiplier = 0.5f + speedRatio * 2.5f;
        }

        TreeNode* tmp = avl.toBinaryTree();
        posCount = layoutGeneralTree(tmp, 34.f, 250.f, 1500.f, 660.f, positions, 64);
        ScratchAVL::freeTN(tmp);

        int rLo = rangeLo.empty() ? 0 : stoi(rangeLo);
        int rHi = rangeHi.empty() ? 999 : stoi(rangeHi);

        for (int i = 0; i < posCount; ++i)
        {
            ScratchAVLNode* an = avl.search(positions[i].value);
            if (an)
            {
                int bf = avl.getBF(an);
                string bStr = (bf > 0 ? "+" : "") + to_string(bf);
                safeCopyStr(positions[i].badge, bStr, sizeof(positions[i].badge));
                positions[i].badgeColor = (abs(bf) <= 1) ? Color(78, 255, 152) : Color(255, 80, 80);
            }
            positions[i].isHighlighted = (positions[i].value >= rLo && positions[i].value <= rHi);
            positions[i].isGreyed = !positions[i].isHighlighted;
            positions[i].isSelected = (positions[i].value == selectedVal);
        }

        window.clear();
        window.draw(bg);

        
        Text hText(font, to_string(avl.treeHeight()), static_cast<unsigned int>(34 * sy));
        hText.setFillColor(Color(40, 50, 70));
        drawCenteredText(window, hText, 175.f * sx, 188.f * sy);

        Text opText(font, to_string(operations), static_cast<unsigned int>(34 * sy));
        opText.setFillColor(Color(30, 80, 140));
        drawCenteredText(window, opText, 496.5f * sx, 188.f * sy);

        bool bal = avl.isBalanced();
        Text balText(font, bal ? "BALANCED" : "IMBALANCE", static_cast<unsigned int>(26 * sy));
        balText.setFillColor(bal ? Color(20, 140, 80) : Color(200, 30, 30));
        drawCenteredText(window, balText, 818.f * sx, 188.f * sy);

        Text banner(font, "AVL TREE SANDBOX | Order-Statistics & Auto-Rebalancing", static_cast<unsigned int>(28 * sy));
        banner.setFillColor(Color(40, 60, 90));
        drawCenteredText(window, banner, 1431.5f * sx, 158.f * sy);

        
        for (int i = 0; i < posCount; ++i)
        {
            if (positions[i].parentId != -1)
            {
                for (int j = 0; j < posCount; ++j)
                {
                    if (positions[j].id == positions[i].parentId)
                    {
                        drawThickLine(window, { positions[i].x * sx, positions[i].y * sy }, { positions[j].x * sx, positions[j].y * sy }, 2.5f * sx, Color(100, 120, 150, 180));
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < posCount; ++i)
        {
            CircleShape c(positions[i].radius * sx);
            c.setOrigin({ positions[i].radius * sx, positions[i].radius * sx });
            c.setPosition({ positions[i].x * sx, positions[i].y * sy });

            if (positions[i].isSelected)
            {
                c.setFillColor(Color(255, 215, 0, 240));
                c.setOutlineThickness(3.f * sx);
                c.setOutlineColor(Color(20, 40, 60));
            }
            else if (positions[i].isHighlighted)
            {
                c.setFillColor(Color(180, 220, 240, 230));
                c.setOutlineThickness(2.f * sx);
                c.setOutlineColor(Color(40, 100, 160));
            }
            else
            {
                c.setFillColor(Color(200, 205, 215, 180));
                c.setOutlineThickness(1.5f * sx);
                c.setOutlineColor(Color(150, 155, 165));
            }
            window.draw(c);

            Text vt(font, to_string(positions[i].value), static_cast<unsigned int>(20 * sx));
            vt.setFillColor(Color(20, 30, 50));
            drawCenteredText(window, vt, positions[i].x * sx, positions[i].y * sy);

            if (positions[i].badge[0] != '\0')
            {
                CircleShape b(11.f * sx);
                b.setOrigin({ 11.f * sx, 11.f * sx });
                b.setPosition({ (positions[i].x + positions[i].radius * 0.7f) * sx, (positions[i].y - positions[i].radius * 0.7f) * sy });
                b.setFillColor(positions[i].badgeColor);
                window.draw(b);

                Text bt(font, positions[i].badge, static_cast<unsigned int>(12 * sx));
                bt.setFillColor(Color::Black);
                drawCenteredText(window, bt, b.getPosition().x, b.getPosition().y);
            }
        }

        
        drawEventLog(window, font, sx, sy, logs, Color(40, 60, 80));

        
        const float rotY[4] = { 771.f, 801.f, 833.f, 864.f };
        CircleShape rDot(5.5f * sx);
        rDot.setOrigin({ 5.5f * sx, 5.5f * sx });
        rDot.setPosition({ 1730.f * sx, rotY[selectedRot] * sy });
        rDot.setFillColor(Color(40, 120, 200));
        window.draw(rDot);

        
        Text inT(font, inputKey + (activeInput == 0 ? "_" : ""), static_cast<unsigned int>(24 * sy));
        inT.setFillColor(Color(20, 60, 100));
        drawCenteredText(window, inT, 1772.f * sx, 564.f * sy);

        
        Text rLoT(font, rangeLo + (activeInput == 1 ? "_" : ""), static_cast<unsigned int>(20 * sy));
        drawCenteredText(window, rLoT, 1634.5f * sx, 1010.5f * sy);

        Text rHiT(font, rangeHi + (activeInput == 2 ? "_" : ""), static_cast<unsigned int>(20 * sy));
        drawCenteredText(window, rHiT, 1798.5f * sx, 1010.5f * sy);

        
        drawSpeedSlider(window, font, sx, sy, speedMultiplier, speedRatio, Color(40, 120, 200), Color(40, 120, 200, 180));

        window.display();
    }

    return "SANDBOX_MAP";
}

static string runSandboxHeap(RenderWindow& window, const Font& font)
{
    Texture bgTex;
    string path = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_MH.png", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_MH.png");
    if (!bgTex.loadFromFile(path)) return "MENU";

    Sprite bg(bgTex);
    ScratchHeap heap;
    int arr[7] = { 90, 75, 80, 50, 60, 45, 70 };
    heap.build(arr, 7);

    SimpleLog logs;
    logs.record("[SANDBOX HEAP] Synchronized Tree and Array views");

    int operations = 0;
    string inputKey = "85";
    int activeInput = -1;

    float speedRatio = 0.5f;
    float speedMultiplier = 1.0f;
    bool draggingSpeed = false;

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>()) { window.close(); return "EXIT GAME"; }
            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape) return "SANDBOX_MAP";
                if (kp->code == Keyboard::Key::Backspace && activeInput == 0 && !inputKey.empty())
                {
                    inputKey.pop_back();
                }
            }
            if (const auto* txt = event->getIf<Event::TextEntered>())
            {
                if (txt->unicode >= '0' && txt->unicode <= '9' && activeInput == 0 && inputKey.size() < 3)
                {
                    inputKey += static_cast<char>(txt->unicode);
                }
            }
            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    Vector2u winSize = window.getSize();
                    Vector2u imgSize = bgTex.getSize();
                    float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
                    float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
                    Vector2i mpos = Mouse::getPosition(window);
                    float mx = static_cast<float>(mpos.x) / sx;
                    float my = static_cast<float>(mpos.y) / sy;

                    if (isInside(mx, my, 25.f, 20.f, 80.f, 70.f)) return "SANDBOX_MAP";

                    
                    if (isInside(mx, my, 965.f, 965.f, 550.f, 65.f))
                    {
                        draggingSpeed = true;
                        speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
                        speedMultiplier = 0.5f + speedRatio * 2.5f;
                    }

                    if (isInside(mx, my, 1718.f, 528.f, 108.f, 72.f)) activeInput = 0;
                    else if (isInside(mx, my, 1559.f, 510.f, 150.f, 109.f))
                    {
                        if (!inputKey.empty())
                        {
                            heap.insert(stoi(inputKey));
                            logs.record("[INSERT] Added " + inputKey + " to Heap");
                            operations++;
                        }
                    }
                    else if (isInside(mx, my, 1562.f, 624.f, 299.f, 101.f))
                    {
                        heap.build(heap.data, heap.size);
                        logs.record("[BUILD] Bottom-up Heap build");
                        operations++;
                    }
                    else if (isInside(mx, my, 1565.f, 735.f, 299.f, 101.f))
                    {
                        heap.siftDown(0, heap.size);
                        logs.record("[HEAPIFY] Restored Max Heap");
                        operations++;
                    }
                    else if (isInside(mx, my, 1568.f, 845.f, 299.f, 101.f))
                    {
                        int v = heap.extractMax();
                        logs.record("[EXTRACT] Removed max " + to_string(v));
                        operations++;
                    }
                    else if (isInside(mx, my, 1571.f, 955.f, 299.f, 101.f))
                    {
                        heap.sort();
                        logs.record("[SORT] Array sorted ascending");
                        operations++;
                    }
                    else if (isInside(mx, my, 37.f, 945.f, 311.f, 117.f))
                    {
                        heap.build(arr, 7);
                        logs.record("[RESET] Reset Max Heap");
                        operations = 0;
                    }
                }
            }
            if (const auto* mbr = event->getIf<Event::MouseButtonReleased>())
            {
                if (mbr->button == Mouse::Button::Left)
                {
                    draggingSpeed = false;
                }
            }
        }

        Vector2u winSize = window.getSize();
        Vector2u imgSize = bgTex.getSize();
        if (winSize.x == 0 || winSize.y == 0) continue;

        float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
        float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
        bg.setScale({ sx, sy });

        Vector2i mpos = Mouse::getPosition(window);
        float mx = static_cast<float>(mpos.x) / sx;
        float my = static_cast<float>(mpos.y) / sy;
        if (draggingSpeed)
        {
            speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
            speedMultiplier = 0.5f + speedRatio * 2.5f;
        }

        window.clear();
        window.draw(bg);

        
        Text hText(font, to_string(heap.size), static_cast<unsigned int>(34 * sy));
        hText.setFillColor(Color::White);
        drawCenteredText(window, hText, 175.f * sx, 188.f * sy);

        Text opText(font, to_string(operations), static_cast<unsigned int>(34 * sy));
        opText.setFillColor(Color(255, 180, 50));
        drawCenteredText(window, opText, 496.5f * sx, 188.f * sy);

        bool isH = heap.isHeap();
        Text isHText(font, isH ? "VALID MAX HEAP" : "NOT A HEAP", static_cast<unsigned int>(26 * sy));
        isHText.setFillColor(isH ? Color(78, 255, 152) : Color(255, 80, 80));
        drawCenteredText(window, isHText, 818.f * sx, 188.f * sy);

        Text banner(font, "MAX HEAP | Synchronized Tree View & Array Strip", static_cast<unsigned int>(28 * sy));
        banner.setFillColor(Color(255, 215, 120));
        drawCenteredText(window, banner, 1431.5f * sx, 158.f * sy);

        
        int hCount = heap.size;
        if (hCount > 0)
        {
            Vector2f heapPositions[32];
            int maxLevel = 0;
            while ((1 << (maxLevel + 1)) - 1 < hCount) maxLevel++;

            float usableW = 1380.f;
            float usableH = 430.f;
            float yStep = (maxLevel > 0) ? (usableH / static_cast<float>(maxLevel)) : 0.f;

            for (int i = 0; i < hCount; ++i)
            {
                int level = 0, temp = i + 1;
                while (temp > 1) { temp >>= 1; level++; }
                int levelStart = (1 << level) - 1;
                int levelCount = 1 << level;
                int idxInLevel = i - levelStart;

                float xStep = usableW / static_cast<float>(levelCount + 1);
                float nx = 60.f + static_cast<float>(idxInLevel + 1) * xStep;
                float ny = 275.f + static_cast<float>(level) * yStep;
                heapPositions[i] = { clampFloat(nx, 70.f, 1490.f), clampFloat(ny, 275.f, 710.f) };
            }

            for (int i = 1; i < hCount; ++i)
            {
                int p = (i - 1) / 2;
                drawThickLine(window, { heapPositions[i].x * sx, heapPositions[i].y * sy },
                                      { heapPositions[p].x * sx, heapPositions[p].y * sy },
                                      2.5f * sx, Color(220, 150, 50, 180));
            }

            for (int i = 0; i < hCount; ++i)
            {
                float radius = (hCount > 7) ? 22.f : 26.f;
                CircleShape c(radius * sx);
                c.setOrigin({ radius * sx, radius * sx });
                c.setPosition({ heapPositions[i].x * sx, heapPositions[i].y * sy });

                if (i == 0)
                {
                    c.setFillColor(Color(255, 190, 0, 240));
                    c.setOutlineThickness(3.f * sx);
                    c.setOutlineColor(Color::White);
                }
                else
                {
                    c.setFillColor(Color(80, 50, 20, 230));
                    c.setOutlineThickness(2.f * sx);
                    c.setOutlineColor(Color(255, 170, 50));
                }
                window.draw(c);

                Text vt(font, to_string(heap.data[i]), static_cast<unsigned int>(20 * sx));
                vt.setFillColor(Color::White);
                drawCenteredText(window, vt, heapPositions[i].x * sx, heapPositions[i].y * sy);
            }
        }

        
        Text arrLabel(font, "Underlying Array View:", static_cast<unsigned int>(18 * sy));
        arrLabel.setFillColor(Color(255, 200, 100));
        arrLabel.setPosition({ 60.f * sx, 770.f * sy });
        window.draw(arrLabel);

        int maxDisplay = 10;
        float cellW = 85.f;
        float cellH = 50.f;
        float startX = 60.f;
        float arrY = 805.f;

        for (int i = 0; i < maxDisplay; ++i)
        {
            float cx = (startX + static_cast<float>(i) * (cellW + 12.f)) * sx;
            float cy = arrY * sy;

            RectangleShape cell({ cellW * sx, cellH * sy });
            cell.setPosition({ cx, cy });

            if (i < heap.size)
            {
                cell.setFillColor(Color(60, 40, 15, 230));
                cell.setOutlineThickness(2.f * sx);
                cell.setOutlineColor(Color(255, 180, 50));
            }
            else
            {
                cell.setFillColor(Color(30, 25, 20, 160));
                cell.setOutlineThickness(1.5f * sx);
                cell.setOutlineColor(Color(80, 70, 60));
            }
            window.draw(cell);

            if (i < heap.size)
            {
                Text vt(font, to_string(heap.data[i]), static_cast<unsigned int>(22 * sy));
                vt.setFillColor(Color(255, 220, 140));
                drawCenteredText(window, vt, cx + cellW * 0.5f * sx, cy + cellH * 0.5f * sy);
            }

            Text idxT(font, "[" + to_string(i) + "]", static_cast<unsigned int>(14 * sy));
            idxT.setFillColor(Color(180, 160, 140));
            drawCenteredText(window, idxT, cx + cellW * 0.5f * sx, cy + cellH * sy + 14.f * sy);
        }

        
        drawEventLog(window, font, sx, sy, logs, Color(255, 220, 160));

        
        Text inT(font, inputKey + (activeInput == 0 ? "_" : ""), static_cast<unsigned int>(24 * sy));
        inT.setFillColor(Color(255, 200, 100));
        drawCenteredText(window, inT, 1772.f * sx, 564.f * sy);

        
        drawSpeedSlider(window, font, sx, sy, speedMultiplier, speedRatio, Color(255, 180, 50), Color(255, 180, 50, 180));

        window.display();
    }

    return "SANDBOX_MAP";
}




string runSandboxMap(RenderWindow& window, const Font& font)
{
    Texture bgTex;
    string path = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_Map.png", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Sandbox_Map.png");
    if (!bgTex.loadFromFile(path)) return "MENU";

    Sprite bg(bgTex);

    const float btnX[4] = { 590.f, 1100.f, 575.f, 1110.f };
    const float btnY[4] = { 285.f,  420.f, 625.f,  760.f };
    const float btnW[4] = { 206.f,  215.f, 200.f,  205.f };
    const float btnH[4] = { 120.f,  120.f, 110.f,  120.f };

    int hovered = -1;

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>()) { window.close(); return "EXIT GAME"; }
            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape) return "MENU";
            }
            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    Vector2u winSize = window.getSize();
                    Vector2u imgSize = bgTex.getSize();
                    float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
                    float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
                    Vector2i mpos = Mouse::getPosition(window);
                    float mx = static_cast<float>(mpos.x) / sx;
                    float my = static_cast<float>(mpos.y) / sy;

                    if (isInside(mx, my, 25.f, 20.f, 80.f, 70.f)) return "MENU";

                    for (int i = 0; i < 4; ++i)
                    {
                        if (isInside(mx, my, btnX[i], btnY[i], btnW[i], btnH[i]))
                        {
                            string subRes;
                            if (i == 0) subRes = runSandboxBinary(window, font);
                            else if (i == 1) subRes = runSandboxBST(window, font);
                            else if (i == 2) subRes = runSandboxAVL(window, font);
                            else if (i == 3) subRes = runSandboxHeap(window, font);

                            if (subRes == "EXIT GAME") return "EXIT GAME";
                            break;
                        }
                    }
                }
            }
        }

        Vector2u winSize = window.getSize();
        Vector2u imgSize = bgTex.getSize();
        if (winSize.x == 0 || winSize.y == 0) continue;

        float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
        float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
        bg.setScale({ sx, sy });

        Vector2i mpos = Mouse::getPosition(window);
        float mx = static_cast<float>(mpos.x) / sx;
        float my = static_cast<float>(mpos.y) / sy;

        hovered = -1;
        for (int i = 0; i < 4; ++i)
        {
            if (isInside(mx, my, btnX[i], btnY[i], btnW[i], btnH[i]))
            {
                hovered = i;
                break;
            }
        }

        window.clear();
        window.draw(bg);

        if (hovered != -1)
        {
            RectangleShape hl({ btnW[hovered] * sx, btnH[hovered] * sy });
            hl.setPosition({ btnX[hovered] * sx, btnY[hovered] * sy });
            hl.setFillColor(Color::Transparent);
            hl.setOutlineThickness(3.f * sx);
            hl.setOutlineColor(Color(78, 255, 152, 230));
            window.draw(hl);
        }

        window.display();
    }

    return "MENU";
}




string runEndless(RenderWindow& window, const Font& font)
{
    Texture bgTex;
    string path = resolveFilePath("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Endless.png", "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\Endless.png");
    if (!bgTex.loadFromFile(path)) return "MENU";

    Sprite bg(bgTex);

    ScratchAVL tree;
    int keys[7] = { 40, 20, 60, 10, 30, 50, 70 };
    for (int k : keys) tree.insert(k, true);

    SimpleLog logs;
    logs.record("[ENDLESS] Survival mode initiated. Keep tree balanced!");

    float survivalTime = 0.f;
    int operations = 0;
    int score = 0;
    int alarmSteps = 0;
    const int alarmCapacity = 5;

    bool isPaused = false;
    bool isGameOver = false;
    float opTimer = 0.f;

    int selectedVal = -1;
    string inputKey = "";
    bool activeInput = false;

    float speedRatio = 0.5f;
    float speedMultiplier = 1.0f;
    bool draggingSpeed = false;

    string playerName = "Agent";

    Clock frameClock;
    GameNodePos positions[64];
    int posCount = 0;

    while (window.isOpen())
    {
        float dt = frameClock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f;

        float opInterval = maxFloat(0.8f, (4.5f - (survivalTime / 30.0f) * 0.6f) / speedMultiplier);
        float currentRate = 1.0f / opInterval;

        if (!isPaused && !isGameOver)
        {
            survivalTime += dt;
            score = static_cast<int>(survivalTime * 25.f) + operations * 120;

            opTimer += dt;
            if (opTimer >= opInterval)
            {
                opTimer = 0.f;
                operations++;
                bool doInsert = (rand() % 100 < 60) || (tree.totalNodes() <= 3);

                if (doInsert && tree.totalNodes() < 18)
                {
                    int val = 10 + (rand() % 89);
                    if (tree.insert(val, true))
                    {
                        logs.record("[INFLUX] Influx room " + to_string(val) + " added");
                    }
                }
                else
                {
                    TreeNode* tmp = tree.toBinaryTree();
                    if (tmp)
                    {
                        int delVal = tmp->value;
                        if (tmp->left && (rand() % 2 == 0)) delVal = tmp->left->value;
                        else if (tmp->right) delVal = tmp->right->value;
                        ScratchAVL::freeTN(tmp);

                        tree.deleteKey(delVal, true);
                        logs.record("[COLLAPSE] Vault room " + to_string(delVal) + " breached!");
                    }
                }

                if (!tree.isBalanced())
                {
                    alarmSteps++;
                    logs.record("[ALARM] Tree balance breached: " + to_string(alarmSteps) + "/" + to_string(alarmCapacity));
                }
                else if (alarmSteps > 0)
                {
                    alarmSteps--;
                }

                if (alarmSteps >= alarmCapacity || tree.totalNodes() == 0 || tree.treeHeight() > 6)
                {
                    isGameOver = true;
                    logs.record("[BREACH] Alarm triggered! Run ended.");
                    triggerAlarm3Seconds(window, &font);
                }
            }
        }

        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>()) { window.close(); return "EXIT GAME"; }
            if (const auto* kp = event->getIf<Event::KeyPressed>())
            {
                if (kp->code == Keyboard::Key::Escape)
                {
                    if (isGameOver) return "LEADERBOARD";
                    return "MENU";
                }
                if (kp->code == Keyboard::Key::Backspace)
                {
                    if (isGameOver && !playerName.empty()) playerName.pop_back();
                    else if (!isGameOver && activeInput && !inputKey.empty()) inputKey.pop_back();
                }
                if (kp->code == Keyboard::Key::Enter)
                {
                    if (isGameOver)
                    {
                        addLBScore(playerName, score, survivalTime);
                        return "LEADERBOARD";
                    }
                }
            }
            if (const auto* txt = event->getIf<Event::TextEntered>())
            {
                if (isGameOver)
                {
                    if ((txt->unicode >= 'a' && txt->unicode <= 'z') ||
                        (txt->unicode >= 'A' && txt->unicode <= 'Z') ||
                        (txt->unicode >= '0' && txt->unicode <= '9'))
                    {
                        if (playerName.size() < 12)
                            playerName += static_cast<char>(txt->unicode);
                    }
                }
                else if (activeInput)
                {
                    if (txt->unicode >= '0' && txt->unicode <= '9' && inputKey.size() < 3)
                    {
                        inputKey += static_cast<char>(txt->unicode);
                    }
                }
            }
            if (const auto* mb = event->getIf<Event::MouseButtonPressed>())
            {
                if (mb->button == Mouse::Button::Left)
                {
                    Vector2u winSize = window.getSize();
                    Vector2u imgSize = bgTex.getSize();
                    float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
                    float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
                    Vector2i mpos = Mouse::getPosition(window);
                    float mx = static_cast<float>(mpos.x) / sx;
                    float my = static_cast<float>(mpos.y) / sy;

                    if (isInside(mx, my, 25.f, 20.f, 80.f, 70.f))
                    {
                        if (isGameOver) return "LEADERBOARD";
                        return "MENU";
                    }

                    
                    if (isInside(mx, my, 965.f, 965.f, 550.f, 65.f))
                    {
                        draggingSpeed = true;
                        speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
                        speedMultiplier = 0.5f + speedRatio * 2.5f;
                    }

                    if (isGameOver)
                    {
                        if (isInside(mx, my, 840.f, 650.f, 240.f, 80.f))
                        {
                            addLBScore(playerName, score, survivalTime);
                            return "LEADERBOARD";
                        }
                    }
                    else
                    {
                        if (isInside(mx, my, 1718.f, 555.f, 115.f, 65.f)) activeInput = true;
                        else if (isInside(mx, my, 1563.f, 540.f, 150.f, 95.f))
                        {
                            int val = !inputKey.empty() ? stoi(inputKey) : (10 + rand() % 89);
                            if (tree.insert(val, true))
                            {
                                operations++;
                                logs.record("[TACTICAL] Counter room " + to_string(val));
                                if (tree.isBalanced() && alarmSteps > 0) alarmSteps--;
                            }
                        }
                        else if (isInside(mx, my, 1563.f, 625.f, 299.f, 99.f))
                        {
                            if (selectedVal != -1)
                            {
                                if (tree.deleteKey(selectedVal, true))
                                {
                                    operations++;
                                    logs.record("[TACTICAL] Pruned room " + to_string(selectedVal));
                                    selectedVal = -1;
                                    if (tree.isBalanced() && alarmSteps > 0) alarmSteps--;
                                }
                            }
                        }
                        else if (isInside(mx, my, 37.f, 945.f, 311.f, 117.f))
                        {
                            tree.clear();
                            for (int k : keys) tree.insert(k, true);
                            survivalTime = 0.f;
                            operations = 0;
                            score = 0;
                            alarmSteps = 0;
                            isGameOver = false;
                            isPaused = false;
                            opTimer = 0.f;
                            logs.record("[RESET] Survival restarted");
                        }
                        else if (isInside(mx, my, 388.f, 945.f, 311.f, 117.f))
                        {
                            isPaused = !isPaused;
                            logs.record(isPaused ? "[PAUSED] Survival clock paused" : "[RESUMED] Alarm active");
                        }

                        for (int i = 0; i < posCount; ++i)
                        {
                            float dx = mx - positions[i].x;
                            float dy = my - positions[i].y;
                            if (dx * dx + dy * dy <= positions[i].radius * positions[i].radius)
                            {
                                selectedVal = positions[i].value;
                                ScratchAVLNode* an = tree.search(positions[i].value);
                                if (an && abs(tree.getBF(an)) > 1)
                                {
                                    tree.root = tree.rebalance(tree.root);
                                    logs.record("[REPAIR] Rotated room " + to_string(positions[i].value));
                                    if (alarmSteps > 0) alarmSteps--;
                                }
                                break;
                            }
                        }
                    }
                }
            }
            if (const auto* mbr = event->getIf<Event::MouseButtonReleased>())
            {
                if (mbr->button == Mouse::Button::Left)
                {
                    draggingSpeed = false;
                }
            }
        }

        Vector2u winSize = window.getSize();
        Vector2u imgSize = bgTex.getSize();
        if (winSize.x == 0 || winSize.y == 0) continue;

        float sx = static_cast<float>(winSize.x) / static_cast<float>(imgSize.x);
        float sy = static_cast<float>(winSize.y) / static_cast<float>(imgSize.y);
        bg.setScale({ sx, sy });

        Vector2i mpos = Mouse::getPosition(window);
        float mx = static_cast<float>(mpos.x) / sx;
        float my = static_cast<float>(mpos.y) / sy;
        if (draggingSpeed)
        {
            speedRatio = clampFloat((mx - 976.f) / 529.f, 0.0f, 1.0f);
            speedMultiplier = 0.5f + speedRatio * 2.5f;
        }

        TreeNode* tmp = tree.toBinaryTree();
        posCount = layoutGeneralTree(tmp, 34.f, 250.f, 1500.f, 660.f, positions, 64);
        ScratchAVL::freeTN(tmp);

        for (int i = 0; i < posCount; ++i)
        {
            ScratchAVLNode* an = tree.search(positions[i].value);
            if (an)
            {
                int bf = tree.getBF(an);
                string bStr = (bf > 0 ? "+" : "") + to_string(bf);
                safeCopyStr(positions[i].badge, bStr, sizeof(positions[i].badge));
                positions[i].badgeColor = (abs(bf) <= 1) ? Color(78, 255, 152) : Color(255, 80, 80);
                positions[i].isViolated = (abs(bf) > 1);
            }
            positions[i].isSelected = (positions[i].value == selectedVal);
        }

        window.clear();
        window.draw(bg);

        
        Text timeT(font, formatTimer(survivalTime), static_cast<unsigned int>(32 * sy));
        timeT.setFillColor(Color::White);
        drawCenteredText(window, timeT, 175.f * sx, 188.f * sy);

        Text opT(font, to_string(operations), static_cast<unsigned int>(34 * sy));
        opT.setFillColor(Color(78, 255, 152));
        drawCenteredText(window, opT, 496.5f * sx, 188.f * sy);

        Text scoreT(font, to_string(score), static_cast<unsigned int>(34 * sy));
        scoreT.setFillColor(Color(255, 215, 0));
        drawCenteredText(window, scoreT, 818.f * sx, 188.f * sy);

        
        float frac = clampFloat(opTimer / opInterval, 0.0f, 1.0f);
        RectangleShape opBar({ 805.f * sx * frac, 25.f * sy });
        opBar.setPosition({ 1026.f * sx, 188.f * sy });
        opBar.setFillColor(Color(78, 255, 152, 220));
        window.draw(opBar);

        int rWhole = static_cast<int>(currentRate);
        int rFrac = static_cast<int>((currentRate - static_cast<float>(rWhole)) * 100.f + 0.5f);
        if (rFrac >= 100) { rWhole++; rFrac -= 100; }
        if (rFrac < 0) rFrac = 0;
        string rFracStr = (rFrac < 10 ? "0" : "") + to_string(rFrac);

        float rem = maxFloat(0.f, opInterval - opTimer);
        int remWhole = static_cast<int>(rem);
        int remFrac = static_cast<int>((rem - static_cast<float>(remWhole)) * 10.f + 0.5f);
        if (remFrac >= 10) { remWhole++; remFrac -= 10; }
        if (remFrac < 0) remFrac = 0;

        string rateBuf = to_string(rWhole) + "." + rFracStr + " ops/sec (Next in " + to_string(remWhole) + "." + to_string(remFrac) + "s)";
        Text rateT(font, rateBuf, static_cast<unsigned int>(18 * sy));
        rateT.setFillColor(Color::White);
        drawCenteredText(window, rateT, 1428.5f * sx, 200.5f * sy);

        
        for (int i = 0; i < posCount; ++i)
        {
            if (positions[i].parentId != -1)
            {
                for (int j = 0; j < posCount; ++j)
                {
                    if (positions[j].id == positions[i].parentId)
                    {
                        drawThickLine(window, { positions[i].x * sx, positions[i].y * sy }, { positions[j].x * sx, positions[j].y * sy }, 2.5f * sx, Color(60, 160, 140, 180));
                        break;
                    }
                }
            }
        }

        
        for (int i = 0; i < posCount; ++i)
        {
            CircleShape c(positions[i].radius * sx);
            c.setOrigin({ positions[i].radius * sx, positions[i].radius * sx });
            c.setPosition({ positions[i].x * sx, positions[i].y * sy });

            if (positions[i].isSelected)
            {
                c.setFillColor(Color(255, 215, 0, 240));
                c.setOutlineThickness(3.f * sx);
                c.setOutlineColor(Color::White);
            }
            else if (positions[i].isViolated)
            {
                c.setFillColor(Color(180, 40, 40, 240));
                c.setOutlineThickness(3.f * sx);
                c.setOutlineColor(Color(255, 100, 100));
            }
            else
            {
                c.setFillColor(Color(20, 80, 70, 230));
                c.setOutlineThickness(2.f * sx);
                c.setOutlineColor(Color(78, 255, 152));
            }
            window.draw(c);

            Text vt(font, to_string(positions[i].value), static_cast<unsigned int>(20 * sx));
            vt.setFillColor(Color::White);
            drawCenteredText(window, vt, positions[i].x * sx, positions[i].y * sy);

            if (positions[i].badge[0] != '\0')
            {
                CircleShape b(11.f * sx);
                b.setOrigin({ 11.f * sx, 11.f * sx });
                b.setPosition({ (positions[i].x + positions[i].radius * 0.7f) * sx, (positions[i].y - positions[i].radius * 0.7f) * sy });
                b.setFillColor(positions[i].badgeColor);
                window.draw(b);

                Text bt(font, positions[i].badge, static_cast<unsigned int>(12 * sx));
                bt.setFillColor(Color::Black);
                drawCenteredText(window, bt, b.getPosition().x, b.getPosition().y);
            }
        }

        
        Text tsNodes(font, "Rooms: " + to_string(tree.totalNodes()), static_cast<unsigned int>(17 * sy));
        tsNodes.setFillColor(Color::White);
        drawCenteredText(window, tsNodes, 1714.5f * sx, 825.f * sy);

        Text tsHeight(font, "Height: " + to_string(tree.treeHeight()) + " (Limit: 6)", static_cast<unsigned int>(17 * sy));
        tsHeight.setFillColor(tree.treeHeight() > 5 ? Color(255, 80, 80) : Color::White);
        drawCenteredText(window, tsHeight, 1714.5f * sx, 865.f * sy);

        bool bal = tree.isBalanced();
        Text tsBal(font, bal ? "Status: STABLE" : "Status: ALERT", static_cast<unsigned int>(17 * sy));
        tsBal.setFillColor(bal ? Color(78, 255, 152) : Color(255, 80, 80));
        drawCenteredText(window, tsBal, 1714.5f * sx, 905.f * sy);

        Text tsAlarm(font, "Alarm: " + to_string(alarmSteps) + " / " + to_string(alarmCapacity), static_cast<unsigned int>(17 * sy));
        tsAlarm.setFillColor(alarmSteps > 2 ? Color(255, 80, 80) : Color(255, 200, 100));
        drawCenteredText(window, tsAlarm, 1714.5f * sx, 945.f * sy);

        
        drawEventLog(window, font, sx, sy, logs, Color(200, 240, 230));

        
        Text inT(font, inputKey + (activeInput ? "_" : ""), static_cast<unsigned int>(24 * sy));
        inT.setFillColor(Color(78, 255, 152));
        drawCenteredText(window, inT, 1772.f * sx, 588.f * sy);

        
        if (isPaused)
        {
            RectangleShape pauseBox({ 311.f * sx, 117.f * sy });
            pauseBox.setPosition({ 388.f * sx, 945.f * sy });
            pauseBox.setFillColor(Color(255, 200, 50, 40));
            pauseBox.setOutlineThickness(2.f * sx);
            pauseBox.setOutlineColor(Color(255, 215, 0, 220));
            window.draw(pauseBox);

            Text pauseT(font, "PAUSED", static_cast<unsigned int>(26 * sy));
            pauseT.setFillColor(Color(255, 215, 0));
            drawCenteredText(window, pauseT, (388.f + 155.5f) * sx, (945.f + 58.5f) * sy);
        }

        
        drawSpeedSlider(window, font, sx, sy, speedMultiplier, speedRatio, Color(78, 255, 152), Color(78, 255, 152, 180));

        
        if (isGameOver)
        {
            RectangleShape shade({ static_cast<float>(winSize.x), static_cast<float>(winSize.y) });
            shade.setFillColor(Color(0, 0, 0, 210));
            window.draw(shade);

            RectangleShape modal({ 700.f * sx, 420.f * sy });
            modal.setOrigin({ 350.f * sx, 210.f * sy });
            modal.setPosition({ winSize.x * 0.5f, winSize.y * 0.5f });
            modal.setFillColor(Color(15, 30, 40, 250));
            modal.setOutlineThickness(3.f * sx);
            modal.setOutlineColor(Color(255, 80, 80));
            window.draw(modal);

            Text goTitle(font, "VAULT HEIST BREACHED!", static_cast<unsigned int>(34 * sy));
            goTitle.setFillColor(Color(255, 80, 80));
            drawCenteredText(window, goTitle, winSize.x * 0.5f, winSize.y * 0.38f);

            Text resText(font, "Time: " + formatTimer(survivalTime) + "  |  Ops: " + to_string(operations) + "  |  Score: " + to_string(score), static_cast<unsigned int>(22 * sy));
            resText.setFillColor(Color(255, 215, 0));
            drawCenteredText(window, resText, winSize.x * 0.5f, winSize.y * 0.45f);

            Text prompt(font, "Attempt Recorded! Press Enter or Submit to View Rankings", static_cast<unsigned int>(20 * sy));
            drawCenteredText(window, prompt, winSize.x * 0.5f, winSize.y * 0.54f);

            RectangleShape submitBtn({ 240.f * sx, 55.f * sy });
            submitBtn.setOrigin({ 120.f * sx, 27.5f * sy });
            submitBtn.setPosition({ winSize.x * 0.5f, winSize.y * 0.64f });
            submitBtn.setFillColor(Color(78, 255, 152));
            window.draw(submitBtn);

            Text subText(font, "VIEW RANKINGS", static_cast<unsigned int>(22 * sy));
            subText.setFillColor(Color::Black);
            drawCenteredText(window, subText, submitBtn.getPosition().x, submitBtn.getPosition().y);
        }

        window.display();
    }

    return "MENU";
}
