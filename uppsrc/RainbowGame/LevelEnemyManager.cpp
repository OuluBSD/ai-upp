#include "LevelEnemyManager.h"
#include <Core/Core.h>

LevelEnemyManager::LevelEnemyManager() : currentWorldRoster(nullptr) {
    // Initialize all world rosters
    // This assumes that WorldType has a way to iterate through its values
    // In a real implementation, you'd iterate through all possible WorldType values
    // For now I'll add a few common ones as examples:
    
    // This would require the actual WorldType enum values
    // worldRosters.Add(WorldType::CASTLE, One<WorldEnemyRoster>(new WorldEnemyRoster(WorldType::CASTLE)));
    // worldRosters.Add(WorldType::FOREST, One<WorldEnemyRoster>(new WorldEnemyRoster(WorldType::FOREST)));
    // worldRosters.Add(WorldType::JUNGLE, One<WorldEnemyRoster>(new WorldEnemyRoster(WorldType::JUNGLE)));
    // ... and so on for other world types
}

void LevelEnemyManager::SetCurrentWorld(WorldType worldType) {
    currentWorldRoster = worldRosters.Get(worldType, nullptr);
    if (currentWorldRoster) {
        currentWorldRoster = worldRosters.Get(worldType).operator->();
    }
}

const EnemyData* LevelEnemyManager::GetEnemyDataForCurrentWorld(const String& enemyTypeName) const {
    if (currentWorldRoster != nullptr) {
        return currentWorldRoster->GetEnemyData(enemyTypeName);
    }
    return nullptr;
}

const EnemyData* LevelEnemyManager::GetEnemyData(WorldType worldType, const String& enemyTypeName) const {
    WorldEnemyRoster* roster = worldRosters.Get(worldType, nullptr);
    if (roster != nullptr) {
        return roster->GetEnemyData(enemyTypeName);
    }
    return nullptr;
}

Vector<String> LevelEnemyManager::GetCurrentWorldDefaultEnemies() const {
    if (currentWorldRoster != nullptr) {
        return currentWorldRoster->GetDefaultEnemies();
    }
    return Vector<String>(); // Return empty vector
}

Vector<One<EnemyBase>> LevelEnemyManager::CreateDefaultEnemiesForWorld(const Vector<SpawnPosition>& spawnPositions) const {
    Vector<One<EnemyBase>> createdEnemies;
    
    if (currentWorldRoster == nullptr) {
        return createdEnemies;
    }
    
    Vector<String> defaultEnemies = currentWorldRoster->GetDefaultEnemies();
    if (defaultEnemies.IsEmpty()) {
        return createdEnemies;
    }
    
    // Cycle through default enemies and create them at each spawn position
    for (int i = 0; i < spawnPositions.GetCount(); i++) {
        SpawnPosition pos = spawnPositions[i];
        String enemyType = defaultEnemies[i % defaultEnemies.GetCount()]; // Cycle through enemy types
        const EnemyData* enemyData = currentWorldRoster->GetEnemyData(enemyType);
        
        if (enemyData != nullptr) {
            // Create enemy with appropriate dimensions based on size
            float sizeFactor = enemyData->GetSize().GetSizeMultiplier();
            
            // This assumes there's a factory method for creating enemies
            // The actual implementation would depend on the EnemyBase constructor and factory methods
            One<EnemyBase> enemy = One<EnemyBase>(
                new EnemyBase(
                    pos.x, pos.y,
                    sizeFactor * 24.0f, // Default size with scaling
                    sizeFactor * 24.0f,
                    *enemyData
                )
            );
            
            if (enemy) {
                createdEnemies.Add(pick(enemy));
            }
        }
    }
    return createdEnemies;
}