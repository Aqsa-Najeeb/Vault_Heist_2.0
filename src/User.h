#pragma once
#include <string>

struct Progress
{
    int stars[20];
};

const char* levelIdAt(int index);
int levelIndexOf(const std::string& id);
void loadProgress(Progress& progress);
void saveProgress(const Progress& progress);
bool isLevelUnlocked(const Progress& progress, int index);
bool isEndlessUnlocked(const Progress& progress);
void recordResult(Progress& progress, const std::string& levelId, int stars);