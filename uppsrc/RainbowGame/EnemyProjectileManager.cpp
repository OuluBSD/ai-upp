#include "EnemyProjectileManager.h"
#include <Core/Core.h>

void EnemyProjectileManager::Update(float delta) {
    // Update all active projectiles and remove inactive ones
    for (int i = projectiles.GetCount() - 1; i >= 0; i--) {
        EnemyProjectile* projectile = projectiles[i];
        if (projectile) {
            projectile->Update(delta);
            if (!projectile->IsActive()) {
                projectiles.Remove(i);
            }
        }
    }
}

void EnemyProjectileManager::CreateProjectile(float x, float y, ProjectileType type, float vx, float vy) {
    One<EnemyProjectile> projectile = One<EnemyProjectile>(new EnemyProjectile(x, y, 8.0f, 8.0f, type, vx, vy));
    projectiles.Add(pick(projectile));
}

void EnemyProjectileManager::CreateProjectile(float x, float y, ProjectileType type,
                                             float directionX, float directionY, float speed) {
    float vx = directionX * speed;
    float vy = directionY * speed;
    One<EnemyProjectile> projectile = One<EnemyProjectile>(new EnemyProjectile(x, y, 8.0f, 8.0f, type, vx, vy));
    projectiles.Add(pick(projectile));
}

bool EnemyProjectileManager::CheckCollisionsWithPlayer(Player* player) {
    if (!player) return false;
    
    for (int i = 0; i < projectiles.GetCount(); i++) {
        EnemyProjectile* projectile = projectiles[i];
        if (projectile && projectile->IsActive()) {
            if (projectile->CheckCollisionWithPlayer(player)) {
                projectile->Deactivate();
                return true; // At least one collision occurred
            }
        }
    }
    return false;
}

void EnemyProjectileManager::Clear() {
    projectiles.Clear();
}

int EnemyProjectileManager::GetActiveProjectileCount() const {
    int count = 0;
    for (int i = 0; i < projectiles.GetCount(); i++) {
        if (projectiles[i] && projectiles[i]->IsActive()) {
            count++;
        }
    }
    return count;
}