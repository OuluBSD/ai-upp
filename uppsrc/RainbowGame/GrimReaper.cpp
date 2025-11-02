#include "GrimReaper.h"
#include <Core/Core.h>
#include <Math/Geometry.h>

GrimReaper::GrimReaper(float x, float y) 
    : EnemyBase(x, y, 48.0f, 64.0f, CreateGrimReaperData())
    , zigzagTimer(0.0f)
    , isChasing(false)
    , musicSwapped(false) {
}

EnemyData GrimReaper::CreateGrimReaperData() {
    // This assumes there's a builder pattern for EnemyData
    // Since we don't have the exact implementation details, I'll create a basic implementation
    return EnemyData(); // Placeholder - would need actual implementation
}

void GrimReaper::ActivateChase() {
    isChasing = true;
}

void GrimReaper::SwapMusic(EnemyAudioManager* audioManager) {
    if (!musicSwapped && audioManager != nullptr) {
        audioManager->PlayMusic("grim-reaper-theme", true); // Play with looping
        musicSwapped = true;
    }
}

void GrimReaper::Update(float delta, Player* player, 
                       TileCollision* collision, 
                       EnemyProjectileManager* projectileManager) {
    // Update animation system
    UpdateAnimation(delta);
    
    if (!isChasing || !player) {
        return; // Don't move until activated
    }
    
    // Update zombie state (though Grim Reaper itself doesn't become zombie)
    UpdateZombie(delta);
    
    // Handle shooting (if applicable)
    UpdateShooting(delta, player, projectileManager);
    
    // Add zigzag pattern to movement
    zigzagTimer += delta * GRIM_REAPER_ZIGZAG_FREQUENCY;
    float zigzagOffset = (float)sin(zigzagTimer) * GRIM_REAPER_ZIGZAG_AMPLITUDE;
    
    // Get player position
    Rect playerBounds = player->GetBounds();
    
    // Calculate target position (player position with zigzag offset)
    float targetX = playerBounds.left + zigzagOffset;
    float targetY = playerBounds.top - 50.0f; // Stay slightly above player
    
    // Calculate direction to target
    float dx = targetX - x;
    float dy = targetY - y;
    
    // Normalize and apply speed
    float distance = (float)sqrt(dx * dx + dy * dy);
    if (distance > 0) {
        dx /= distance;
        dy /= distance;
        // Apply movement speed
        velocity.x = dx * GRIM_REAPER_SPEED;
        velocity.y = dy * GRIM_REAPER_SPEED;
    } else {
        // If already at target, stop moving
        velocity.x = 0;
        velocity.y = 0;
    }
    
    // Apply movement to position
    x += velocity.x * delta;
    y += velocity.y * delta;
    
    // Update animation state to chasing
    // Assuming there's a method to update animation state
    // getAnimationSystem()->setState(AnimationState::ZOMBIE); // Use zombie state for menacing appearance
}

bool GrimReaper::HasCaughtPlayer(Player* player) {
    if (!isChasing || !player) return false;
    
    Rect thisBounds = GetBounds();
    Rect playerBounds = player->GetBounds();
    
    return thisBounds.Intersects(playerBounds);
}