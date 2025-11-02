#include "EnemyProjectile.h"
#include <Core/Core.h>

EnemyProjectile::EnemyProjectile(float x, float y, float width, float height, 
                                ProjectileType type, float vx, float vy)
    : type(type), active(true), lifetime(0.0f) {
    bounds.Set(x, y, x + width, y + height);
    velocity.x = vx;
    velocity.y = vy;
    
    // Set lifetime based on projectile type
    switch (type) {
        case ProjectileType::BULLET:
            maxLifetime = 2.0f; // Standard bullet lifetime
            break;
        case ProjectileType::LIGHTNING:
            maxLifetime = 1.0f; // Lightning dissipates quickly
            break;
        case ProjectileType::WATER:
            maxLifetime = 3.0f; // Water projectile
            break;
        case ProjectileType::FIRE:
            maxLifetime = 2.5f; // Fire projectile
            break;
        default:
            maxLifetime = 2.0f;
    }
}

void EnemyProjectile::Update(float delta) {
    // Update position based on velocity
    bounds.left += velocity.x * delta;
    bounds.right += velocity.x * delta;
    bounds.top += velocity.y * delta;
    bounds.bottom += velocity.y * delta;
    
    // Update lifetime
    lifetime += delta;
    
    // Deactivate if lifetime exceeded
    if (lifetime > maxLifetime) {
        active = false;
    }
    
    // Boundary checks - deactivate if outside screen bounds
    // Assuming screen dimensions are 800x600
    if (bounds.left < -50 || bounds.right > 850 || bounds.top < -50 || bounds.bottom > 650) {
        active = false;
    }
}

bool EnemyProjectile::CheckCollisionWithPlayer(Player* player) {
    if (!active || !player) return false;
    
    // This assumes Player has a GetBounds() method
    // The actual implementation would depend on how Player class is defined
    // For now, we'll use a placeholder
    Rect playerBounds = player->GetBounds(); // This would need to match the actual Player class
    return active && bounds.Intersects(playerBounds);
}

void EnemyProjectile::Deactivate() {
    active = false;
}

Vector<float> EnemyProjectile::GetColor() const {
    Vector<float> color(4);
    switch (type) {
        case ProjectileType::BULLET:
            color[0] = 0.8f; // Red
            color[1] = 0.8f; // Green
            color[2] = 0.2f; // Blue
            color[3] = 1.0f; // Alpha
            break;
        case ProjectileType::LIGHTNING:
            color[0] = 0.9f; // Red
            color[1] = 0.9f; // Green
            color[2] = 0.2f; // Blue
            color[3] = 1.0f; // Alpha
            break;
        case ProjectileType::WATER:
            color[0] = 0.2f; // Red
            color[1] = 0.5f; // Green
            color[2] = 0.9f; // Blue
            color[3] = 1.0f; // Alpha
            break;
        case ProjectileType::FIRE:
            color[0] = 0.9f; // Red
            color[1] = 0.4f; // Green
            color[2] = 0.2f; // Blue
            color[3] = 1.0f; // Alpha
            break;
        default:
            color[0] = 0.8f; // Red
            color[1] = 0.8f; // Green
            color[2] = 0.2f; // Blue
            color[3] = 1.0f; // Alpha
            break;
    }
    return color;
}