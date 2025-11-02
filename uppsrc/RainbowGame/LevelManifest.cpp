#include "LevelManifest.h"

LevelManifest::LevelManifest() {
    // Load the level manifest
    LoadManifest();
}

LevelManifest::~LevelManifest() {
    // Cleanup
}

LevelManifest::LevelInfo LevelManifest::GetLevel(const String& id) const {
    for (const LevelInfo& level : levels) {
        if (level.id == id) {
            return level;
        }
    }
    
    // Return an empty level info if not found
    return LevelInfo();
}

Vector<LevelManifest::LevelInfo> LevelManifest::GetLevelsForWorld(int world) const {
    Vector<LevelInfo> worldLevels;
    for (const LevelInfo& level : levels) {
        if (level.world == world) {
            worldLevels.Add(level);
        }
    }
    return worldLevels;
}

bool LevelManifest::IsLevelUnlocked(const String& id) const {
    for (const LevelInfo& level : levels) {
        if (level.id == id) {
            return level.isUnlocked;
        }
    }
    return false;  // Level not found, so it's locked by default
}

void LevelManifest::UnlockLevel(const String& id) {
    for (LevelInfo& level : levels) {
        if (level.id == id) {
            level.isUnlocked = true;
            break;
        }
    }
}

int LevelManifest::GetTotalWorlds() const {
    int maxWorld = 0;
    for (const LevelInfo& level : levels) {
        if (level.world > maxWorld) {
            maxWorld = level.world;
        }
    }
    return maxWorld;
}

int LevelManifest::GetTotalLevels() const {
    return levels.GetCount();
}

void LevelManifest::LoadManifest() {
    // Clear existing levels
    levels.Clear();
    
    // Add sample levels - in a real implementation, you would load from a data file
    LevelInfo level1;
    level1.id = "world1_level1";
    level1.name = "Rainbow Bridge";
    level1.description = "Cross the rainbow bridge to reach the castle";
    level1.world = 1;
    level1.level = 1;
    level1.isUnlocked = true;  // First level is always unlocked
    
    LevelInfo level2;
    level2.id = "world1_level2";
    level2.name = "Cloud Hopping";
    level2.description = "Hop from cloud to cloud to avoid the gaps";
    level2.world = 1;
    level2.level = 2;
    level2.isUnlocked = false;  // Locked until previous level is completed
    
    LevelInfo level3;
    level3.id = "world1_level3";
    level3.name = "Treasure Hunt";
    level3.description = "Find all the hidden treasures in the level";
    level3.world = 1;
    level3.level = 3;
    level3.isUnlocked = false;
    
    levels.Add(level1);
    levels.Add(level2);
    levels.Add(level3);
    
    // Add more levels as needed...
}