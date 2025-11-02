#ifndef RAINBOWGAME_LEVELMANIFEST_H
#define RAINBOWGAME_LEVELMANIFEST_H

#include <Core/Core.h>

using namespace Upp;

class LevelManifest {
public:
    LevelManifest();
    ~LevelManifest();
    
    struct LevelInfo {
        String id;
        String name;
        String description;
        int world;
        int level;
        bool isUnlocked;
        Vector<String> requiredCollectibles;
    };
    
    Vector<LevelInfo> GetLevels() const { return levels; }
    LevelInfo GetLevel(const String& id) const;
    Vector<LevelInfo> GetLevelsForWorld(int world) const;
    
    bool IsLevelUnlocked(const String& id) const;
    void UnlockLevel(const String& id);
    
    int GetTotalWorlds() const;
    int GetTotalLevels() const;
    
    void LoadManifest();
    
private:
    Vector<LevelInfo> levels;
};

#endif