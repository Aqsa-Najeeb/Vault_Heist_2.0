#include <SFML/Graphics.hpp>
#include <string>

using namespace std;
using namespace sf;

static string levelImagePath(const string& levelId)
{
	return "C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\images\\level_" + levelId + ".png";
}

string runLevel(RenderWindow& window, const string& levelId)
{
	Texture backgroundTexture;

    if (!backgroundTexture.loadFromFile(levelImagePath(levelId)))
    {
        return "Can't_load_image";
    }

    Sprite background(backgroundTexture);

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
            {
                window.close();
                return "Exit";
            }

            if (const auto* keyPressed = event->getIf<Event::KeyPressed>())
            {
                if (keyPressed->code == Keyboard::Key::Escape)
                {
                    return "Back";
                }
            }
        }

        const Vector2u windowSize = window.getSize();
        const Vector2u imageSize = backgroundTexture.getSize();

        if (windowSize.x == 0 || windowSize.y == 0)
        {
            continue;
        }

        const float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(imageSize.x);
        const float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(imageSize.y);

        background.setScale({ scaleX, scaleY });

        window.clear();
        window.draw(background);

        window.display();
    }

    return "Exit";
}