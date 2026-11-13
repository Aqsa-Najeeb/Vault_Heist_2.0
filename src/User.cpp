#include "User.h"
#include <fstream>

using namespace std;

const char* levelIdAt(int index)
{
    static const char* ids[20] =
    {
        "1.1", "1.2", "1.3", "1.4", "1.5",
        "2.1", "2.2", "2.3", "2.4", "2.5",
        "3.1", "3.2", "3.3", "3.4", "3.5",
        "4.1", "4.2", "4.3", "4.4", "4.5"
    };

    return ids[index];
}

int levelIndexOf(const string& id)
{
    for (int i = 0; i < 20; ++i)
    {
        if (id == levelIdAt(i))
            return i;
    }
    return -1;
}

void loadProgress(Progress& progress)
{
    for (int i = 0; i < 20; ++i)
        progress.stars[i] = 0;

    ifstream in("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\data\\progress.sav");
    if (!in)
        return;

    for (int i = 0; i < 20; ++i)
    {
        int value = 0;
        in >> value;
        if (value < 0) value = 0;
        if (value > 3) value = 3;
        progress.stars[i] = value;
    }
}

void saveProgress(const Progress& progress)
{
    ofstream out("C:\\Users\\jensc\\repos\\Vault2\\Vault_Heist_2.0\\data\\progress.sav");
    
    for (int i = 0; i < 20; ++i)
    {
        out << progress.stars[i] << "\n";
    }
        
}

bool isLevelUnlocked(const Progress& progress, int index)
{
    return (index == 0 || progress.stars[index - 1] >= 1);
}

bool isEndlessUnlocked(const Progress& progress)
{
    return (progress.stars[14] >= 1);
}

void recordResult(Progress& progress, const string& levelId, int stars)
{
    int index = levelIndexOf(levelId);
    
    if (index < 0)
    {
        return;
    }

    if (stars > progress.stars[index])
    {
        progress.stars[index] = stars;
    }
       
    saveProgress(progress);
}