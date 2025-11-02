#include "EnemySpawner.h"
#include <Core/Core.h>
#include <Math/Algorithms.h>

EnemySpawner::EnemySpawner(Vector<One<EnemyBase>>& enemyContainer) 
    : enemyContainer(enemyContainer), spawnTimer(0.0f), spawnInterval(3.0f) {
}

void EnemySpawner::UpdateSpawning(EnemyBase* enemy, Player* player, float delta) {
    if (!enemy || !player) return;
    
    // Check if the enemy has a spawn type defined
    EnemySpawnType spawnType = enemy->GetSpawnType();
    if (spawnType == EnemySpawnType::NONE) { // Assuming there's a NONE enum value
        // No spawn type, nothing to spawn
        return;
    }
    
    // Check if the enemy is alive and not deactivated
    if (!enemy->IsDead() && !enemy->IsDeactivated()) {
        // Update the spawn timer
        spawnTimer += delta;
        
        // Check if enough time has passed for a spawn attempt
        if (spawnTimer >= spawnInterval) {
            // Reset the timer with some variance to make it less predictable
            spawnTimer = (float)(Random(1.0) * 1.5f); // Random value between 0 and 1.5
            
            // There's still a chance that spawning doesn't happen (to make it less frequent)
            if (Random(1.0f) < 0.3f) { // Only 30% chance to actually spawn when timer expires
                SpawnEnemyFromType(spawnType, enemy, player);
            }
        }
    }
}

void EnemySpawner::SpawnEnemyFromType(EnemySpawnType spawnType, EnemyBase* parentEnemy, Player* player) {
    if (!parentEnemy || !player) return;
    
    // Get the spawn type string - this assumes EnemySpawnType has a method to get a string representation
    // For now, I'll assume we have a way to get the string name of the enemy type to spawn
    String typeToSpawn = spawnType.ToString(); // This method would need to be implemented
    
    if (typeToSpawn.IsEmpty()) {
        return; // Nothing to spawn
    }
    
    // Find the appropriate enemy data for the type to spawn
    const EnemyData* enemyData = GetEnemyDataForType(typeToSpawn);
    if (!enemyData) {
        // If we can't find the specific enemy data, we might try a fallback
        // For now just return
        return;
    }
    
    // Calculate spawn position near the parent enemy
    Rect bounds = parentEnemy->GetBounds();
    float spawnX = bounds.left + (float)(Random(1.0) * 32 - 16); // Random offset from parent
    float spawnY = bounds.top + (float)(Random(1.0) * 32 - 16);  // Random offset from parent
    
    // Create the enemy based on its data
    // This assumes we have an EnemyFactory-like approach
    One<EnemyBase> newEnemy = One<EnemyBase>(
        new EnemyBase(spawnX, spawnY,
                      enemyData->GetSize().GetSizeMultiplier() * 32.0f, // Use size multiplier to determine dimensions
                      enemyData->GetSize().GetSizeMultiplier() * 32.0f,
                      *enemyData)
    );
    
    if (newEnemy) {
        enemyContainer.Add(pick(newEnemy));
    }
}

const EnemyData* EnemySpawner::GetEnemyDataForType(const String& type) {
    // This would typically be a lookup in a registry of all possible enemy types
    // For now, we'll check against the known enemy types in EnemyDefinitions
    String lowerType = ToLower(type);
    
    if (lowerType == "casta-kun") {
        // Assuming EnemyDefinitions is a class that provides static EnemyData objects
        // return &EnemyDefinitions::CASTA_KUN;  // This would need to be implemented
        return nullptr; // Placeholder
    } else if (lowerType == "pet-kun") {
        // return &EnemyDefinitions::PET_KUN;
        return nullptr; // Placeholder
    } else if (lowerType == "pianon") {
        // return &EnemyDefinitions::PIANON;
        return nullptr; // Placeholder
    } else if (lowerType == "vio-kun") {
        // return &EnemyDefinitions::VIO_KUN;
        return nullptr; // Placeholder
    } else if (lowerType == "apple-head") {
        // return &EnemyDefinitions::APPLE_HEAD;
        return nullptr; // Placeholder
    } else {
        // In a real implementation, you might have a registry of all possible enemy types
        // For now, return null if the type is unknown
        return nullptr;
    }
}

void EnemySpawner::UpdateAll(Vector<One<EnemyBase>>& enemies, Player* player, float delta) {
    // Iterate through all enemies to check for spawning
    for (int i = 0; i < enemies.GetCount(); i++) {
        EnemyBase* enemy = enemies[i];
        if (enemy) {
            UpdateSpawning(enemy, player, delta);
        }
    }
}