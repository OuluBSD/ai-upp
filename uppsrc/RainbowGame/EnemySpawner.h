#ifndef _RainbowGame_EnemySpawner_h_
#define _RainbowGame_EnemySpawner_h_

#include <Core/Core.h>
#include "EnemyBase.h"
#include "Player.h"
#include "EnemyData.h"
#include "EnemyDefinitions.h"  // Assuming this contains enemy data definitions
#include "EnemySpawnType.h"    // Assuming this exists

using namespace Upp;

// Forward declarations
class EnemyBase;
class Player;
class EnemyData;

/**
* Handles enemy spawning based on spawn types defined in enemy data
*/
class EnemySpawner {
private:
    Vector<One<EnemyBase>>& enemyContainer;
    float spawnTimer;
    float spawnInterval; // Spawn attempts every 3 seconds

public:
    EnemySpawner(Vector<One<EnemyBase>>& enemyContainer);
    
    /**
    * Checks if an enemy should spawn other enemies and handles the spawning
    * @param enemy The enemy that might spawn others
    * @param player The player for positioning reference
    * @param delta Time elapsed since last frame
    */
    void UpdateSpawning(EnemyBase* enemy, Player* player, float delta);
    
    /**
    * Updates all spawners in the container
    * @param enemies The array of enemies to check for spawning
    * @param player The player for positioning reference
    * @param delta Time elapsed since last frame
    */
    void UpdateAll(Vector<One<EnemyBase>>& enemies, Player* player, float delta);

private:
    /**
    * Attempts to spawn an enemy based on the spawn type
    * @param spawnType The type of enemy to spawn
    * @param parentEnemy The enemy that is spawning
    * @param player The player for positioning reference
    */
    void SpawnEnemyFromType(EnemySpawnType spawnType, EnemyBase* parentEnemy, Player* player);
    
    /**
    * Retrieves the appropriate EnemyData for the given type string
    * @param type The enemy type string
    * @return The corresponding EnemyData or null if not found
    */
    const EnemyData* GetEnemyDataForType(const String& type);
};

#endif