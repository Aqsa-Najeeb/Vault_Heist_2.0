#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

string runMainMenu(RenderWindow& window);

int main()
{
    RenderWindow window(VideoMode({ 1648, 928 }), "Vault Heist");

    string choice = runMainMenu(window);

    if (choice == "CAMPAIGN MAP")
    {
        // CampaignMap.cpp
    }
    else if (choice == "ENDLESS")
    {
        // EndlessMode.cpp
    }
    else if (choice == "SANDBOX")
    {
        // Sandbox.cpp
    }
    else if (choice == "LEADERBOARD")
    {
        // Call Leaderboard.cpp
    }
    else if (choice == "SETTINGS")
    {
        // Settings.cpp
    }
    else if (choice == "EXIT GAME")
    {
        window.close();
    }
}