#ifndef _RainbowGame_EnemyElementInteractionSystem_h_
#define _RainbowGame_EnemyElementInteractionSystem_h_

#include <Core/Core.h>
#include "EnemyBase.h"
#include "Player.h"
#include "EnemyElementType.h"  // Assuming this exists
#include "EnemyMovementType.h"  // Assuming this exists

using namespace Upp;

// Enum for visual effects associated with elements
enum class ElementVisualEffect {
    NONE,
    WATER_RIPPLE,
    LIGHTNING_BOLT,
    FIRE_SPARK,
    EARTH_QUAKE,
    AIR_SWIRL
};

class EnemyElementInteractionSystem {
public:
    EnemyElementInteractionSystem() = default;
    ~EnemyElementInteractionSystem() = default;
    
    // Process world-specific element interactions for enemies
    void ProcessElementInteractions(EnemyBase* enemy, Player* player, float deltaTime);
    
    // Get a visual effect type based on the element
    ElementVisualEffect GetVisualEffectForElement(EnemyElementType elementType);

private:
    // Process water element interactions for Music Star enemies
    void ProcessWaterElementInteraction(EnemyBase* enemy, Player* player, float deltaTime);
    
    // Process lightning element interactions for Woods Star enemies
    void ProcessLightningElementInteraction(EnemyBase* enemy, Player* player, float deltaTime);
    
    // Process fire element interactions
    void ProcessFireElementInteraction(EnemyBase* enemy, Player* player, float deltaTime);
    
    // Process earth element interactions
    void ProcessEarthElementInteraction(EnemyBase* enemy, Player* player, float deltaTime);
    
    // Process air element interactions
    void ProcessAirElementInteraction(EnemyBase* enemy, Player* player, float deltaTime);
    
    // Helper method to calculate distance between two rectangles
    float CalculateDistance(const Rectf& rect1, const Rectf& rect2);
};

#endif