#ifndef _RainbowGame_WorldEnemyRoster_h_
#define _RainbowGame_WorldEnemyRoster_h_

#include <Core/Core.h>
#include "EnemyData.h"  // Assuming this exists
#include "WorldType.h"  // Assuming this exists

using namespace Upp;

// Forward declarations
class EnemyData;

/**
* Enum for world types
*/
enum class WorldType {
    MUSIC_STAR,    // World 1
    WOODS_STAR,    // World 2
    OCEAN_STAR,    // World 3
    MACHINE_STAR,  // World 4
    GAMBLE_STAR,   // World 5
    CLOUDS_STAR,   // World 6
    GIANT_STAR,    // World 7
    RAINBOW_STAR    // World 8
};

/**
* Represents a level-specific enemy roster for a particular world
*/
class WorldEnemyRoster {
private:
    WorldType worldType;
    VectorMap<String, One<EnemyData>> enemyTypes;
    Vector<String> defaultEnemies;  // Default enemies for this world
    Vector<String> bossEnemies;     // Boss enemies for this world

public:
    WorldEnemyRoster(WorldType worldType);
    
    WorldType GetWorldType() const { return worldType; }
    
    const Vector<String>& GetDefaultEnemies() const { return defaultEnemies; }
    
    const Vector<String>& GetBossEnemies() const { return bossEnemies; }
    
    const EnemyData* GetEnemyData(const String& enemyTypeName) const;
    
    bool ContainsEnemyType(const String& enemyTypeName) const;
    
    const Vector<String> GetEnemyTypeNames() const;

private:
    void InitializeWorldEnemies();
};

#endif