#include "GroundPatrolEnemy.h"
#include <Core/Core.h>
#include <Math/Algorithms.h>

GroundPatrolEnemy::GroundPatrolEnemy(float x, float y, float width, float height, 
                                    const EnemyData& enemyData, const String& initialDirection)
    : EnemyBase(x, y, width, height, enemyData)
    , direction(initialDirection == "right" || initialDirection == "RIGHT" ? 1 : -1)
    , patrolTimer(0.0f)
    , isHopping(false)
    , hopTimer(0.0f)
    , hopVelocityY(250.0f) { // Vertical velocity when hopping
    patrolInterval = 2.0f + (float)(Random(2.0f)); // Random interval between 2-4 seconds
    patrolTimer = patrolInterval;
}

void GroundPatrolEnemy::Update(float delta, Player* player, 
                              TileCollision* collision, 
                              EnemyProjectileManager* projectileManager) {
    // Update animation system
    UpdateAnimation(delta);
    
    // Update zombie state if applicable
    UpdateZombie(delta);
    
    // Handle shooting
    UpdateShooting(delta, player, projectileManager);
    
    // Apply horizontal movement based on patrol direction
    float speedMultiplier = GetMovementSpeed(); // 1-3 scale
    float baseSpeed = 40.0f; // Base speed factor for patrollers
    if (IsZombie()) {
        speedMultiplier *= 1.5f; // Zombies are 50% faster
    }
    float effectiveSpeed = baseSpeed * speedMultiplier;
    
    // Apply movement direction
    velocity.x = direction * effectiveSpeed;
    
    // Handle hopping behavior
    HandleHopping(delta, collision);
    
    // Apply gravity
    velocity.y -= 900.0f * delta;
    
    // Update position
    Rect bounds = GetBounds();
    x += velocity.x * delta;
    y += velocity.y * delta;
    
    // Check collision with tiles X-axis - this assumes there's a method to handle collision
    // This is a simplified version - the actual collision system would be more complex
    if (collision) {
        // In a real implementation, this would check if the movement causes collision
        // and bounce off walls by changing direction
        // For now, I'm leaving this as a placeholder since we don't have the full collision system
    }
    
    // Check collision with tiles Y-axis
    // In a real implementation, this would check if the movement causes collision
    // and stop vertical velocity when landing
    
    // Update patrol timer
    patrolTimer -= delta;
    if (patrolTimer <= 0) {
        ConsiderPatrolChange();
        patrolTimer = patrolInterval;
    }
}

void GroundPatrolEnemy::HandleHopping(float delta, TileCollision* collision) {
    if (isHopping) {
        hopTimer -= delta;
        if (hopTimer <= 0) {
            isHopping = false;
        }
    } else {
        // Check if on ground and ready to hop
        // Assuming that when velocity.y is close to 0, we're on the ground
        if (abs(velocity.y) < 10.0f && Random(1.0f) < 0.01f) { // 1% chance per frame to hop
            velocity.y = hopVelocityY;
            isHopping = true;
            hopTimer = 0.5f; // Keep hop state for a time
        }
    }
}

void GroundPatrolEnemy::ConsiderPatrolChange() {
    // Sometimes change direction randomly
    if (Random(1.0f) < 0.4f) { // 40% chance to change direction
        direction *= -1;
    }
    
    // Sometimes hop more frequently
    if (Random(1.0f) < 0.3f) { // 30% chance to initiate hop
        // Only if on ground
        if (abs(velocity.y) < 10.0f) { // Check if on ground
            velocity.y = hopVelocityY * (IsZombie() ? 1.3f : 1.0f); // Zombies hop higher
            isHopping = true;
            hopTimer = 0.5f;
        }
    }
}