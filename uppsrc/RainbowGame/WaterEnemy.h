#ifndef _RainbowGame_WaterEnemy_h_
#define _RainbowGame_WaterEnemy_h_

#include <Core/Core.h>
#include "EnemyBase.h"
#include "Player.h"
#include "EnemyData.h"
#include "TileCollision.h"
#include "EnemyProjectileManager.h"

using namespace Upp;

// Forward declarations
class Player;
class TileCollision;
class EnemyProjectileManager;

/**
* Specialized enemy for water mechanics in Music Star
* These enemies have special interactions with blue balls (water elements)
*/
class WaterEnemy : public EnemyBase {
private:
    float specialWaterTimer;
    static constexpr float SPECIAL_WATER_INTERVAL = 2.0f; // Interval for special water behavior

public:
    WaterEnemy(float x, float y, float width, float height, const EnemyData& enemyData);
    
    virtual void Update(float delta, Player* player, 
                       TileCollision* collision, 
                       EnemyProjectileManager* projectileManager) override;

private:
    /**
    * Update special water behavior
    */
    void UpdateWaterBehavior(float delta, Player* player);
    
    /**
    * Helper method to calculate distance between two rectangles
    */
    float CalculateDistance(const Rect& rect1, const Rect& rect2) const;
};

#endif