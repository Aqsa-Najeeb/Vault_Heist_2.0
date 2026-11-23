#pragma once
#include <SFML/Graphics.hpp>
#include <string>
using namespace std;
using namespace sf;


struct GameSettings
{
    bool musicEnabled = true;
    bool unlockAll = false;
    bool alarmSoundEnabled = true;
};

void initGameAudio();
void setGameMusicEnabled(bool enabled);
void setGameAlarmEnabled(bool enabled);
void triggerAlarm3Seconds(sf::RenderWindow& window, const sf::Font* font = nullptr);

GameSettings& getGameSettings();
void loadGameSettings();
void saveGameSettings();

string runSettings(RenderWindow& window, const Font& font);
string runLeaderboard(RenderWindow& window, const Font& font);
string runSandboxMap(RenderWindow& window, const Font& font);
string runEndless(RenderWindow& window, const Font& font);
