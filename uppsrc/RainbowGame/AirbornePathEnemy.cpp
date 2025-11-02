#include "AirbornePathEnemy.h"
#include <Core/Core.h>

// Constructor
AirbornePathEnemy::AirbornePathEnemy(float x, float y, float width, float height,
                                     const EnemyData& enemyData, MotionPath path)
    : EnemyBase(x, y, width, height, enemyData)
    , motionPath(path)
    , pathTimer(0.0f)
{
    // Set initial directions based on path type
    switch (path) {
        case MotionPath::HORIZONTAL:
            horizontalDirection = Random(2) > 0.5 ? 1 : -1;  // Random initial direction
            verticalDirection = 0;  // No vertical movement
            break;
        case MotionPath::VERTICAL:
            horizontalDirection = 0;  // No horizontal movement
            verticalDirection = Random(2) > 0.5 ? 1 : -1;  // Random initial direction
            break;
        case MotionPath::DIAGONAL:
            horizontalDirection = Random(2) > 0.5 ? 1 : -1;  // Random initial direction
            verticalDirection = Random(2) > 0.5 ? 1 : -1;    // Random initial direction
            break;
    }
    
    pathChangeInterval = 3.0f + (float)(Random(2.0f)); // 3-5 seconds interval
    pathTimer = (float)(Random(pathChangeInterval));    // Random initial delay
}

// Update method
void AirbornePathEnemy::Update(float delta, Player* player,
                              TileCollision* collision,
                              EnemyProjectileManager* projectileManager) {
    // Update animation system (assuming this exists in base class)
    UpdateAnimation(delta);
    
    // Update zombie state if applicable (assuming this exists in base class)
    UpdateZombie(delta);
    
    // Handle shooting (assuming this exists in base class)
    UpdateShooting(delta, player, projectileManager);
    
    // Calculate movement based on path type
    float speedMultiplier = GetMovementSpeed(); // 1-3 scale from base class
    float baseSpeed = 60.0f; // Base speed for flying enemies
    
    if (IsZombie()) {
        speedMultiplier *= 1.5f; // Zombies are 50% faster
    }
    
    float effectiveSpeed = baseSpeed * speedMultiplier;
    
    // Reset velocity each frame to prevent accumulation
    GetVelocity().x = 0;
    GetVelocity().y = 0;
    
    // Apply movement based on path
    switch (motionPath) {
        case MotionPath::HORIZONTAL:
            GetVelocity().x = horizontalDirection * effectiveSpeed;
            break;
        case MotionPath::VERTICAL:
            GetVelocity().y = verticalDirection * effectiveSpeed;
            break;
        case MotionPath::DIAGONAL:
            GetVelocity().x = horizontalDirection * effectiveSpeed * 0.7f; // Slightly slower diagonally
            GetVelocity().y = verticalDirection * effectiveSpeed * 0.7f;
            break;
    }
    
    // Update position
    Pointf bounds = GetBounds();
    x += GetVelocity().x * delta;
    y += GetVelocity().y * delta;
    
    // Update path timer
    pathTimer += delta;
    if (pathTimer >= pathChangeInterval) {
        ConsiderPathChange();
        pathTimer = 0.0f;
    }
    
    // Boundary checks to keep enemy in play area
    // Assuming screen width of 640 and height of 480
    float screenWidth = 640.0f;
    float screenHeight = 480.0f;
    
    if (x < 0) {
        x = 0;
        if (motionPath == MotionPath::HORIZONTAL || motionPath == MotionPath::DIAGONAL) {
            horizontalDirection = 1;
        }
    } else if (x + width > screenWidth) {
        x = screenWidth - width;
        if (motionPath == MotionPath::HORIZONTAL || motionPath == MotionPath::DIAGONAL) {
            horizontalDirection = -1;
        }
    }
    
    if (y < 0) {
        y = 0;
        if (motionPath == MotionPath::VERTICAL || motionPath == MotionPath::DIAGONAL) {
            verticalDirection = 1;
        }
    } else if (y + height > screenHeight) {
        y = screenHeight - height;
        if (motionPath == MotionPath::VERTICAL || motionPath == MotionPath::DIAGONAL) {
            verticalDirection = -1;
        }
    }
}

// Consider changing the movement path
void AirbornePathEnemy::ConsiderPathChange() {
    float changeChance = IsZombie() ? 0.7f : 0.3f; // Zombies change path more frequently
    
    if (Random(1.0f) < changeChance) {
        // Randomly change direction for current path
        switch (motionPath) {
            case MotionPath::HORIZONTAL:
                horizontalDirection *= -1;
                break;
            case MotionPath::VERTICAL:
                verticalDirection *= -1;
                break;
            case MotionPath::DIAGONAL:
                // Change both directions with some randomness
                if (Random(1.0f) > 0.5f) horizontalDirection *= -1;
                if (Random(1.0f) > 0.5f) verticalDirection *= -1;
                break;
        }
    }
    
    // Occasionally change path type entirely (less frequent)
    if (Random(1.0f) < 0.2f) { // 20% chance to change path type
        int newPathIdx = (int)(Random(3.0f)); // 3 possible path types
        
        // Ensure the new index is valid
        MotionPath newPaths[] = {MotionPath::HORIZONTAL, MotionPath::VERTICAL, MotionPath::DIAGONAL};
        motionPath = newPaths[newPathIdx];
        
        // Adjust directions based on new path
        switch (motionPath) {
            case MotionPath::HORIZONTAL:
                horizontalDirection = Random(1.0f) > 0.5f ? 1 : -1;
                verticalDirection = 0;
                break;
            case MotionPath::VERTICAL:
                horizontalDirection = 0;
                verticalDirection = Random(1.0f) > 0.5f ? 1 : -1;
                break;
            case MotionPath::DIAGONAL:
                horizontalDirection = Random(1.0f) > 0.5f ? 1 : -1;
                verticalDirection = Random(1.0f) > 0.5f ? 1 : -1;
                break;
        }
    }
}