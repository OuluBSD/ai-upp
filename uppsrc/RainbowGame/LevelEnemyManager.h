#ifndef _RainbowGame_LevelEnemyManager_h_
#define _RainbowGame_LevelEnemyManager_h_

#include <Core/Core.h>
#include "WorldEnemyRoster.h"  // Assuming this exists
#include "EnemyData.h"         // Assuming this exists
#include "EnemyBase.h"         // Assuming this exists
#include "WorldType.h"         // Assuming this exists

using namespace Upp;

// Forward declarations
class WorldEnemyRoster;
class EnemyData;
class EnemyBase;
enum class WorldType;

/**
* Data structure to hold spawn positions
*/
struct SpawnPosition {
    float x, y;
    
    SpawnPosition(float x, float y) : x(x), y(y) {}
};

/**
* Manages all world-specific enemy rosters and handles level-specific spawning
*/
class LevelEnemyManager {
private:
    VectorMap<WorldType, One<WorldEnemyRoster>> worldRosters;
    WorldEnemyRoster* currentWorldRoster;

public:
    LevelEnemyManager();
    
    /**
    * Sets the current world for the level, affecting which enemies can spawn
    * @param worldType The world type for the current level
    */
    void SetCurrentWorld(WorldType worldType);
    
    /**
    * Gets the current world roster
    * @return The current world roster or null if none set
    */
    WorldEnemyRoster* GetCurrentWorldRoster() const { return currentWorldRoster; }
    
    /**
    * Gets an enemy data for a specific enemy type in the current world
    * @param enemyTypeName The name of the enemy type
    * @return The enemy data or null if not available in current world
    */
    const EnemyData* GetEnemyDataForCurrentWorld(const String& enemyTypeName) const;
    
    /**
    * Gets an enemy data for a specific world type and enemy type
    * @param worldType The world type
    * @param enemyTypeName The name of the enemy type
    * @return The enemy data or null if not available in the specified world
    */
    const EnemyData* GetEnemyData(WorldType worldType, const String& enemyTypeName) const;
    
    /**
    * Gets the default enemies for the current world
    * @return Array of default enemy type names
    */
    Vector<String> GetCurrentWorldDefaultEnemies() const;
    
    /**
    * Creates a set of default enemies for the current world at specified positions
    * @param spawnPositions Array of positions where enemies should be spawned
    * @return Array of created enemies
    */
    Vector<One<EnemyBase>> CreateDefaultEnemiesForWorld(const Vector<SpawnPosition>& spawnPositions) const;
};

#endif