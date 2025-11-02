#ifndef _RainbowGame_EnemyProjectile_h_
#define _RainbowGame_EnemyProjectile_h_

#include <Core/Core.h>
#include "Player.h"  // Assuming this exists

using namespace Upp;

// Forward declaration
class Player;

/**
* Represents an enemy projectile (bullet, lightning, etc.)
*/
enum class ProjectileType {
    BULLET,     // Standard bullet
    LIGHTNING,  // Lightning bolt
    WATER,      // Water projectile for Music Star
    FIRE        // Fire projectile for some enemies
};

class EnemyProjectile {
private:
    Rect bounds;
    Pointf velocity;
    ProjectileType type;
    bool active;
    float lifetime;
    float maxLifetime;

public:
    EnemyProjectile(float x, float y, float width, float height, 
                   ProjectileType type, float vx, float vy);
    
    void Update(float delta);
    
    /**
    * Check collision with player
    */
    bool CheckCollisionWithPlayer(Player* player);
    
    /**
    * Get bounds for collision detection
    */
    const Rect& GetBounds() const { return bounds; }
    
    /**
    * Get velocity for movement
    */
    const Pointf& GetVelocity() const { return velocity; }
    
    ProjectileType GetType() const { return type; }
    
    bool IsActive() const { return active; }
    
    void Deactivate();
    
    /**
    * Get color values for rendering based on projectile type
    */
    Vector<float> GetColor() const;
    
    // Accessor methods for position
    float GetX() const { return bounds.left; }
    float GetY() const { return bounds.top; }
    
    // Setter methods for position
    void SetPosition(float x, float y) { 
        float width = bounds.GetWidth();
        float height = bounds.GetHeight();
        bounds.Set(x, y, x + width, y + height);
    }
};

#endif