#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "User.h"

using namespace std;
using namespace sf;

string runMainMenu(RenderWindow& window);
string runCampaignMap(RenderWindow& window, const Progress& progress);

int main()
{
    RenderWindow window(VideoMode({ 1920, 1080 }), "Vault Heist", State::Fullscreen);

    Progress progress;
    loadProgress(progress);

    string screen = "MENU";

    while (window.isOpen())
    {
        if (screen == "MENU")
        {
            screen = runMainMenu(window);
        }
        else if (screen == "CAMPAIGN MAP")
        {
            screen = runCampaignMap(window, progress);

            if (screen == "BACK")
                screen = "MENU";
        }
        else if (screen.rfind("LEVEL ", 0) == 0)
        {
            screen = "CAMPAIGN MAP";
        }
        else if (screen == "ENDLESS" || screen == "SANDBOX" ||
            screen == "LEADERBOARD" || screen == "SETTINGS")
        {
            screen = "MENU";
        }
        else if (screen == "ASSET_ERROR")
        {
            cerr << "Could not load an image. Check the file paths." << endl;
            window.close();
        }
        else if (screen == "EXIT GAME")
        {
            window.close();
        }
        else
        {
            screen = "MENU";
        }
    }

    return 0;
}