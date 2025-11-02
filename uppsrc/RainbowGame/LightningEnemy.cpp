#include "LightningEnemy.h"
#include <Core/Core.h>
#include <Math/Algorithms.h>
#include <cmath>

LightningEnemy::LightningEnemy(float x, float y, float width, float height, const EnemyData& enemyData)
    : EnemyBase(x, y, width, height, enemyData)
    , electricalCharge(0.0f)
    , dischargeTimer(0.0f) {
}

void LightningEnemy::Update(float delta, Player* player, 
                           TileCollision* collision, 
                           EnemyProjectileManager* projectileManager) {
    // Update animation system
    UpdateAnimation(delta);
    
    // Update zombie state if applicable
    UpdateZombie(delta);
    
    // Handle shooting (lightning enemies might shoot differently)
    UpdateLightningShooting(delta, player, projectileManager);
    
    // Handle special lightning behavior
    UpdateLightningBehavior(delta, player, projectileManager);
    
    // Apply movement based on the specific enemy type
    float speedMultiplier = GetMovementSpeed(); // 1-3 scale
    float baseSpeed = 45.0f; // Base speed factor for lightning enemies
    if (IsZombie()) {
        speedMultiplier *= 1.5f; // Zombies are 50% faster
    }
    float effectiveSpeed = baseSpeed * speedMultiplier;
    
    // Basic sideways movement
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

void LightningEnemy::UpdateLightningShooting(float delta, Player* player, EnemyProjectileManager* projectileManager) {
    if (!player || !projectileManager) return;
    
    // Lightning enemies might have different shooting patterns
    // when they have accumulated electrical charge
    shootTimer -= delta;
    if (shootTimer <= 0.0f) {
        // Check if player is within shooting range
        Rect playerBounds = player->GetBounds();
        Rect enemyBounds = GetBounds();
        float distanceToPlayer = abs(playerBounds.left - enemyBounds.left);
        if (distanceToPlayer < 200.0f) { // Within 200 pixels
            if (electricalCharge > 5.0f) {
                // Shoot a powerful lightning bolt when charged
                ShootPowerfulLightning(player, projectileManager);
                electricalCharge = 0.0f; // Discharge after powerful shot
            } else {
                // Regular lightning shot
                ShootAtPlayer(player, projectileManager);
                electricalCharge += 1.0f; // Build up charge
            }
        }
        // Reset timer with some randomness
        shootTimer = shootCooldown + (float)(Random(5.0f));
    }
}

void LightningEnemy::ShootPowerfulLightning(Player* player, EnemyProjectileManager* projectileManager) {
    if (!player || !projectileManager) return;
    
    Rect playerBounds = player->GetBounds();
    Rect enemyBounds = GetBounds();
    
    // Calculate direction to player
    float dirX = playerBounds.left - enemyBounds.left;
    float dirY = playerBounds.top - enemyBounds.top;
    float distance = (float)sqrt(dirX * dirX + dirY * dirY);
    
    if (distance > 0) {
        dirX /= distance;
        dirY /= distance;
    } else {
        // If player is exactly on same position, shoot horizontally
        dirX = 1.0f;
        dirY = 0.0f;
    }
    
    // Calculate shot position (center of enemy)
    float shotX = enemyBounds.left + enemyBounds.Width() / 2.0f;
    float shotY = enemyBounds.top + enemyBounds.Height() / 2.0f;
    
    // Fire a more powerful projectile with higher speed
    float speed = 150.0f + (GetMovementSpeed() * 50.0f); // Faster than normal
    
    // Create a lightning projectile
    projectileManager->CreateProjectile(shotX, shotY, ProjectileType::LIGHTNING,
                                       dirX, dirY, speed);
    
    // Create additional chain lightning effects
    CreateChainLightning(shotX, shotY, projectileManager);
}

void LightningEnemy::CreateChainLightning(float sourceX, float sourceY, EnemyProjectileManager* projectileManager) {
    if (!projectileManager) return;
    
    // For now, create a secondary lightning in a random direction
    float angle = (float)(Random(1.0) * M_PI * 2); // Random angle
    float dirX = (float)cos(angle);
    float dirY = (float)sin(angle);
    float speed = 80.0f; // Slower chain lightning
    
    projectileManager->CreateProjectile(sourceX, sourceY, ProjectileType::LIGHTNING,
                                       dirX, dirY, speed);
}

void LightningEnemy::UpdateLightningBehavior(float delta, Player* player, EnemyProjectileManager* projectileManager) {
    dischargeTimer += delta;
    if (dischargeTimer >= DISCHARGE_INTERVAL) {
        // Randomly discharge accumulated electricity
        if (electricalCharge > 3.0f && Random(1.0f) < 0.3f) {
            // Create a small electrical discharge around the enemy
            Rect bounds = GetBounds();
            float dischargeX = bounds.left + (float)(Random(1.0) * bounds.Width());
            float dischargeY = bounds.top + (float)(Random(1.0) * bounds.Height());
            
            // Create a random electrical discharge
            float angle = (float)(Random(1.0) * M_PI * 2);
            float dirX = (float)cos(angle);
            float dirY = (float)sin(angle);
            float speed = 60.0f;
            
            // Assuming there's a projectile manager to create projectiles
            if (projectileManager) {
                projectileManager->CreateProjectile(dischargeX, dischargeY,
                                                   ProjectileType::LIGHTNING,
                                                   dirX, dirY, speed);
            }
            
            electricalCharge *= 0.7f; // Reduce charge after discharge
        }
        
        // Lightning enemies might accumulate charge over time when not shooting
        if (electricalCharge < 10.0f) {
            electricalCharge += 0.5f;
        }
        
        dischargeTimer = 0.0f;
    }
}