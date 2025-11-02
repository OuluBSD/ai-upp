#ifndef _RainbowGame_LightningEnemy_h_
#define _RainbowGame_LightningEnemy_h_

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
* Specialized enemy for lightning mechanics in Woods Star
* These enemies have special electrical properties and behaviors
*/
class LightningEnemy : public EnemyBase {
private:
    float electricalCharge; // Accumulated electrical charge
    float dischargeTimer;
    static constexpr float DISCHARGE_INTERVAL = 2.0f; // Interval for electrical discharge

public:
    LightningEnemy(float x, float y, float width, float height, const EnemyData& enemyData);
    
    virtual void Update(float delta, Player* player, 
                       TileCollision* collision, 
                       EnemyProjectileManager* projectileManager) override;
    
    /**
    * Get the current electrical charge level
    */
    float GetElectricalCharge() const { return electricalCharge; }

private:
    /**
    * Update lightning shooting with electrical properties
    */
    void UpdateLightningShooting(float delta, Player* player, EnemyProjectileManager* projectileManager);
    
    /**
    * Shoot a powerful lightning bolt when electrically charged
    */
    void ShootPowerfulLightning(Player* player, EnemyProjectileManager* projectileManager);
    
    /**
    * Create chain lightning that can jump to nearby enemies or player
    */
    void CreateChainLightning(float sourceX, float sourceY, EnemyProjectileManager* projectileManager);
    
    /**
    * Update special lightning behavior
    */
    void UpdateLightningBehavior(float delta, Player* player, EnemyProjectileManager* projectileManager);
};

#endif