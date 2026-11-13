#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

static bool isInside(float x, float y, float left, float top, float width, float height)
{
    return (x >= left && x <= left + width && y >= top && y <= top + height);
}

string runMainMenu(RenderWindow& window)
{
    Texture backgroundTexture;
    
    if (!backgroundTexture.loadFromFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\main_menu.png"))
    {
        return "ASSET_ERROR";
    }

    Sprite background(backgroundTexture);

    const string menuItems[6] =
    {
        "CAMPAIGN MAP",
        "ENDLESS",
        "SANDBOX",
        "LEADERBOARD",
        "SETTINGS",
        "EXIT GAME"
    };

    const float buttonX[6] = { 1446.f, 1450.f, 1450.f, 1450.f, 1450.f, 1450.f };
    const float buttonY[6] = { 140.f,  280.f,  435.f,  591.f,  747.f,  902.f };
    const float buttonW[6] = { 423.f,  248.f,  261.f,  391.f,  257.f,  300.f };
    const float buttonH[6] = { 95.f,   95.f,   96.f,   95.f,   95.f,   95.f };

    int hoveredItem = -1;
    bool showDebugBoxes = false;

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
                    return "EXIT GAME";
                if (keyPressed->code == Keyboard::Key::F1)
                    showDebugBoxes = !showDebugBoxes;
            }

            if (const auto* mousePressed =
                event->getIf<Event::MouseButtonPressed>())
            {
                if (mousePressed->button == Mouse::Button::Left &&
                    hoveredItem != -1)
                {
                    return menuItems[hoveredItem];
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

        const Vector2i mousePosition = sf::Mouse::getPosition(window);
        const float mouseX = static_cast<float>(mousePosition.x) / scaleX;
        const float mouseY = static_cast<float>(mousePosition.y) / scaleY;

        hoveredItem = -1;

        for (int i = 0; i < 6; ++i)
        {
            if (isInside(mouseX, mouseY, buttonX[i], buttonY[i], buttonW[i], buttonH[i]))
            {
                hoveredItem = i;
                break;
            }
        }

        window.clear();
        window.draw(background);

        if (showDebugBoxes)
        {
            for (int i = 0; i < 6; ++i)
            {
                RectangleShape box;
                box.setPosition({ buttonX[i] * scaleX, buttonY[i] * scaleY });
                box.setSize({ buttonW[i] * scaleX, buttonH[i] * scaleY });
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
            outline.setSize({ buttonW[i] * scaleX, buttonH[i] * scaleY });
            outline.setFillColor(sf::Color::Transparent);
            outline.setOutlineThickness(3.f);
            outline.setOutlineColor(Color(255, 218, 145, 230));

            window.draw(outline);
        }

        window.display();
    }

    return "EXIT GAME";
}