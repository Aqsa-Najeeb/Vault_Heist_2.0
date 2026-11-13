#include <SFML/Graphics.hpp>
#include <string>
#include <cmath>
#include "User.h"

using namespace std;
using namespace sf;

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
        star.setFillColor(Color(255, 218, 145));
    }
    else
    {
        star.setFillColor(Color::Transparent);
        star.setOutlineThickness(1.f);
        star.setOutlineColor(Color(140, 150, 160, 200));
    }

    window.draw(star);
}

string runCampaignMap(RenderWindow& window, const Progress& progress)
{
    Texture backgroundTexture;

    if (!backgroundTexture.loadFromFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\campaign_map.png"))
    {
        return "ASSET_ERROR";
    }

    Sprite background(backgroundTexture);

    const float buttonX[21] =
    {
        409.f, 409.f, 409.f, 409.f, 409.f,
        715.f, 715.f, 715.f, 715.f, 715.f,
        1021.f, 1021.f, 1021.f, 1021.f, 1021.f,
        1326.f, 1326.f, 1326.f, 1326.f, 1326.f,
        1021.f
    };
    const float buttonY[21] =
    {
        383.f, 467.f, 552.f, 637.f, 722.f,
        380.f, 464.f, 549.f, 634.f, 718.f,
        383.f, 467.f, 552.f, 637.f, 722.f,
        383.f, 467.f, 552.f, 637.f, 722.f,
        811.f
    };
    const float buttonW = 185.f;
    const float buttonH = 74.f;

    int hoveredItem = -1;
    bool showDebugBoxes = false;
    bool unlockAll = false;

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
            {
                window.close();
                return "EXIT GAME";
            }

            if (const auto* keyPressed = event->getIf<Event::KeyPressed>())
            {
                if (keyPressed->code == Keyboard::Key::Escape)
                    return "BACK";
                
                if (keyPressed->code == Keyboard::Key::F1)
                    showDebugBoxes = !showDebugBoxes;
                
                if (keyPressed->code == Keyboard::Key::F2)
                    unlockAll = !unlockAll;
            }

            if (const auto* mousePressed = event->getIf<Event::MouseButtonPressed>())
            {
                if (mousePressed->button == Mouse::Button::Left && hoveredItem != -1)
                {
                    if (hoveredItem == 20)
                        return "ENDLESS";
                    return string("LEVEL ") + levelIdAt(hoveredItem);
                }
            }
        }

        const Vector2u windowSize = window.getSize();
        const Vector2u imageSize = backgroundTexture.getSize();

        if (windowSize.x == 0 || windowSize.y == 0 || imageSize.x == 0 || imageSize.y == 0)
        {
            continue;
        }

        const float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(imageSize.x);
        const float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(imageSize.y);

        background.setScale({ scaleX, scaleY });

        const Vector2i mousePosition = Mouse::getPosition(window);
        const float mouseX = static_cast<float>(mousePosition.x) / scaleX;
        const float mouseY = static_cast<float>(mousePosition.y) / scaleY;

        bool unlocked[21];
        
        for (int i = 0; i < 20; ++i)
            unlocked[i] = unlockAll || isLevelUnlocked(progress, i);
        
        unlocked[20] = unlockAll || isEndlessUnlocked(progress);

        hoveredItem = -1;
        
        for (int i = 0; i < 21; ++i)
        {
            if (unlocked[i] && isInside(mouseX, mouseY, buttonX[i], buttonY[i], buttonW, buttonH))
            {
                hoveredItem = i;
                break;
            }
        }

        window.clear();
        window.draw(background);

        for (int i = 0; i < 21; ++i)
        {
            const float px = buttonX[i] * scaleX;
            const float py = buttonY[i] * scaleY;
            const float pw = buttonW * scaleX;
            const float ph = buttonH * scaleY;

            if (!unlocked[i])
            {
                RectangleShape shade;
                shade.setPosition({ px, py });
                shade.setSize({ pw, ph });
                shade.setFillColor(Color(0, 0, 0, 160));
                window.draw(shade);
                continue;
            }

            if (i < 20)
            {
                for (int s = 0; s < 3; ++s)
                {
                    const float cx = px + pw - (52.f - 18.f * static_cast<float>(s)) * scaleX;
                    const float cy = py + ph / 2.f;
                    drawStar(window, cx, cy, 8.f * scaleX, progress.stars[i] > s);
                }
            }
        }

        if (showDebugBoxes)
        {
            for (int i = 0; i < 21; ++i)
            {
                RectangleShape box;
                box.setPosition({ buttonX[i] * scaleX, buttonY[i] * scaleY });
                box.setSize({ buttonW * scaleX, buttonH * scaleY });
                box.setFillColor(Color::Transparent);
                box.setOutlineThickness(1.f);
                box.setOutlineColor(Color(255, 0, 0, 200));
                window.draw(box);
            }
        }

        if (hoveredItem != -1)
        {
            const int i = hoveredItem;

            RectangleShape outline;
            outline.setPosition({ buttonX[i] * scaleX, buttonY[i] * scaleY });
            outline.setSize({ buttonW * scaleX, buttonH * scaleY });
            outline.setFillColor(Color::Transparent);
            outline.setOutlineThickness(3.f);
            outline.setOutlineColor(Color(255, 218, 145, 230));
            window.draw(outline);
        }

        window.display();
    }

    return "EXIT GAME";
}