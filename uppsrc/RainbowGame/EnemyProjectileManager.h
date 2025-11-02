#ifndef _RainbowGame_EnemyProjectileManager_h_
#define _RainbowGame_EnemyProjectileManager_h_

#include <Core/Core.h>
#include "EnemyProjectile.h"
#include "Player.h"  // Assuming this exists

using namespace Upp;

// Forward declaration
class Player;

/**
* Manages all enemy projectiles in the game
*/
class EnemyProjectileManager {
private:
    Vector<One<EnemyProjectile>> projectiles;

public:
    EnemyProjectileManager() = default;
    ~EnemyProjectileManager() = default;
    
    void Update(float delta);
    
    /**
    * Create a projectile based on the enemy's projectile type
    */
    void CreateProjectile(float x, float y, ProjectileType type, float vx, float vy);
    
    /**
    * Create a projectile using the enemy's configured projectile type
    */
    void CreateProjectile(float x, float y, ProjectileType type,
                         float directionX, float directionY, float speed);
    
    /**
    * Check for collisions between any active projectiles and the player
    */
    bool CheckCollisionsWithPlayer(Player* player);
    
    /**
    * Get all active projectiles for rendering
    */
    const Vector<One<EnemyProjectile>>& GetProjectiles() const { return projectiles; }
    
    /**
    * Clear all projectiles
    */
    void Clear();
    
    // Get the count of active projectiles
    int GetActiveProjectileCount() const;
};

#endif