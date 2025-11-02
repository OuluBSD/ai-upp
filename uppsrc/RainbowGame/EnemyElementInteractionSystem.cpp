#include "EnemyElementInteractionSystem.h"
#include <Core/Core.h>
#include <Math/Geometry.h>

void EnemyElementInteractionSystem::ProcessElementInteractions(EnemyBase* enemy, Player* player, float deltaTime) {
    if (!enemy || !player) return;
    
    EnemyElementType elementType = enemy->GetEnemyData().GetElementType();
    
    switch (elementType) {
        case EnemyElementType::WATER:
            ProcessWaterElementInteraction(enemy, player, deltaTime);
            break;
        case EnemyElementType::LIGHTNING:
            ProcessLightningElementInteraction(enemy, player, deltaTime);
            break;
        case EnemyElementType::FIRE:
            ProcessFireElementInteraction(enemy, player, deltaTime);
            break;
        case EnemyElementType::EARTH:
            ProcessEarthElementInteraction(enemy, player, deltaTime);
            break;
        case EnemyElementType::AIR:
            ProcessAirElementInteraction(enemy, player, deltaTime);
            break;
        default:
            // For NORMAL or unhandled elements, do nothing special
            break;
    }
}

void EnemyElementInteractionSystem::ProcessWaterElementInteraction(EnemyBase* enemy, Player* player, float deltaTime) {
    if (!enemy || !player) return;
    
    // Water enemies might have special behavior when near water or blue balls
    // Check if player has blue balls (water element)
    if (player->GetBlueBalls() > 0) {
        // Water enemies might be attracted to or repelled by blue balls
        Rectf playerBounds = player->GetBounds();
        Rectf enemyBounds = enemy->GetBounds();
        float distanceToPlayer = CalculateDistance(playerBounds, enemyBounds);
        
        // If close to player, water enemies might behave differently
        if (distanceToPlayer < 100.0f) {
            // Example: Increase movement speed temporarily
            // This would be implemented in the enemy's movement logic
            enemy->SetMovementSpeedMultiplier(1.2f);  // 20% speed boost
        }
    }
}

void EnemyElementInteractionSystem::ProcessLightningElementInteraction(EnemyBase* enemy, Player* player, float deltaTime) {
    if (!enemy || !player) return;
    
    // Lightning enemies might have electrical properties
    // Make lightning enemies shoot more frequently when near other lightning enemies
    EnemyMovementType movementType = enemy->GetMovementType();
    if (movementType == EnemyMovementType::IDLE || movementType == EnemyMovementType::SIDEWAYS) {
        // Increase shooting frequency for stationary lightning enemies
        // This would be handled by modifying the enemy's shooting timer
        enemy->SetShootingFrequencyMultiplier(1.5f);  // 50% more frequent shooting
    }
}

void EnemyElementInteractionSystem::ProcessFireElementInteraction(EnemyBase* enemy, Player* player, float deltaTime) {
    if (!enemy || !player) return;
    
    // Fire element behavior - could be additional damage or different movement
    // when player has fire-related power-ups
    // Implementation would depend on specific game mechanics
}

void EnemyElementInteractionSystem::ProcessEarthElementInteraction(EnemyBase* enemy, Player* player, float deltaTime) {
    if (!enemy || !player) return;
    
    // Earth element behavior - could affect collision detection or movement speed
    // on different terrain types
    // Implementation would depend on specific game mechanics
}

void EnemyElementInteractionSystem::ProcessAirElementInteraction(EnemyBase* enemy, Player* player, float deltaTime) {
    if (!enemy || !player) return;
    
    // Air element behavior - perhaps flying enemies are more affected
    // Could modify jump height, fall speed, or movement in air
    // Implementation would depend on specific game mechanics
}

float EnemyElementInteractionSystem::CalculateDistance(const Rectf& rect1, const Rectf& rect2) {
    // Calculate center points
    float centerX1 = rect1.left + (rect1.right - rect1.left) / 2.0f;
    float centerY1 = rect1.top + (rect1.bottom - rect1.top) / 2.0f;
    float centerX2 = rect2.left + (rect2.right - rect2.left) / 2.0f;
    float centerY2 = rect2.top + (rect2.bottom - rect2.top) / 2.0f;
    
    // Calculate distance
    float dx = centerX1 - centerX2;
    float dy = centerY1 - centerY2;
    
    return sqrt(dx * dx + dy * dy);
}

ElementVisualEffect EnemyElementInteractionSystem::GetVisualEffectForElement(EnemyElementType elementType) {
    switch (elementType) {
        case EnemyElementType::WATER:
            return ElementVisualEffect::WATER_RIPPLE;
        case EnemyElementType::LIGHTNING:
            return ElementVisualEffect::LIGHTNING_BOLT;
        case EnemyElementType::FIRE:
            return ElementVisualEffect::FIRE_SPARK;
        case EnemyElementType::EARTH:
            return ElementVisualEffect::EARTH_QUAKE;
        case EnemyElementType::AIR:
            return ElementVisualEffect::AIR_SWIRL;
        default:
            return ElementVisualEffect::NONE;
    }
}