#include "WaterEnemy.h"
#include <Core/Core.h>
#include <Math/Algorithms.h>
#include <cmath>

WaterEnemy::WaterEnemy(float x, float y, float width, float height, const EnemyData& enemyData)
    : EnemyBase(x, y, width, height, enemyData)
    , specialWaterTimer(0.0f) {
}

void WaterEnemy::Update(float delta, Player* player, 
                       TileCollision* collision, 
                       EnemyProjectileManager* projectileManager) {
    // Update animation system
    UpdateAnimation(delta);
    
    // Update zombie state if applicable
    UpdateZombie(delta);
    
    // Handle shooting
    UpdateShooting(delta, player, projectileManager);
    
    // Handle special water element behavior
    UpdateWaterBehavior(delta, player);
    
    // Apply movement based on the specific enemy type
    // This is just a basic implementation - specific subclasses would override for detailed behavior
    float speedMultiplier = GetMovementSpeed(); // 1-3 scale
    float baseSpeed = 50.0f; // Base speed factor
    if (IsZombie()) {
        speedMultiplier *= 1.5f; // Zombies are 50% faster
    }
    float effectiveSpeed = baseSpeed * speedMultiplier;
    
    // Basic sideways movement (this would be more specific in actual enemy classes)
    velocity.x = effectiveSpeed; // Move right
    
    // Apply gravity
    velocity.y -= 900.0f * delta;
    
    // Update position
    Rect bounds = GetBounds();
    x += velocity.x * delta;
    y += velocity.y * delta;
    
    // Check collision with tiles X-axis
    // In a real implementation, this would check if the movement causes collision
    // and bounce off walls by reversing velocity
    // For now, I'll leave this as a placeholder since we don't have the full collision system
    
    // Check collision with tiles Y-axis
    // In a real implementation, this would check if the movement causes collision
    // and stop vertical velocity when landing
}

void WaterEnemy::UpdateWaterBehavior(float delta, Player* player) {
    if (!player) return;
    
    specialWaterTimer += delta;
    if (specialWaterTimer >= SPECIAL_WATER_INTERVAL) {
        // Check for nearby blue balls (water elements)
        if (player->GetBlueBalls() > 0) {
            // Water enemies might react to blue balls
            // This could be an attraction or repulsion effect
            Rect playerBounds = player->GetBounds();
            Rect enemyBounds = GetBounds();
            float distanceToPlayer = CalculateDistance(playerBounds, enemyBounds);
            if (distanceToPlayer < 150.0f) {
                // When near player with blue balls, water enemies might change behavior
                // Example: move faster, change direction, or shoot more frequently
                velocity.x *= 1.5f; // Temporary speed boost
            }
        }
        specialWaterTimer = 0.0f;
    }
}

float WaterEnemy::CalculateDistance(const Rect& rect1, const Rect& rect2) const {
    // Calculate center points
    float centerX1 = rect1.left + rect1.Width() / 2.0f;
    float centerY1 = rect1.top + rect1.Height() / 2.0f;
    float centerX2 = rect2.left + rect2.Width() / 2.0f;
    float centerY2 = rect2.top + rect2.Height() / 2.0f;
    
    // Calculate distance
    float dx = centerX1 - centerX2;
    float dy = centerY1 - centerY2;
    return (float)sqrt(dx * dx + dy * dy);
}