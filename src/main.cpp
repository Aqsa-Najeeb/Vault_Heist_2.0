#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "User.h"
#include "GameModes.h"

using namespace std;
using namespace sf;

string runMainMenu(RenderWindow& window);
string runCampaignMap(RenderWindow& window, const Progress& progress);
string runLevel(RenderWindow& window, const string& levelId, Progress& progress);

int main()
{
    RenderWindow window(VideoMode({ 1920, 1080 }), "Vault Heist", State::Fullscreen);

    Progress progress;
    loadProgress(progress);

    loadGameSettings();
    initGameAudio();

    Font font;
    if (!font.openFromFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\fonts\\perandory-semi-condensed.otf"))
    {
        if (!font.openFromFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\fonts\\CinzelDecorative-Regular.ttf"))
        {
            (void)font.openFromFile("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\src\\assets\\fonts\\CinzelDecorative-Regular.ttf");
        }
    }

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

            if (screen == "Back")
                screen = "MENU";

            while (screen.rfind("LEVEL ", 0) == 0 && window.isOpen())
            {
                string levelId = screen.substr(6);
                string levelResult = runLevel(window, levelId, progress);

                if (levelResult == "Exit")
                {
                    screen = "EXIT GAME";
                    break;
                }
                else if (levelResult == "Next")
                {
                    int idx = levelIndexOf(levelId);
                    if (idx >= 0 && idx + 1 < 20)
                    {
                        screen = string("LEVEL ") + levelIdAt(idx + 1);
                        continue;
                    }
                    screen = "CAMPAIGN MAP";
                    break;
                }
                else
                {
                    screen = "CAMPAIGN MAP";
                    break;
                }
            }
        }
        else if (screen.rfind("LEVEL ", 0) == 0)
        {
            screen = "CAMPAIGN MAP";
        }
        else if (screen == "ENDLESS")
        {
            screen = runEndless(window, font);
        }
        else if (screen == "SANDBOX" || screen == "SANDBOX_MAP")
        {
            screen = runSandboxMap(window, font);
        }
        else if (screen == "LEADERBOARD")
        {
            screen = runLeaderboard(window, font);
        }
        else if (screen == "SETTINGS")
        {
            screen = runSettings(window, font);
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
