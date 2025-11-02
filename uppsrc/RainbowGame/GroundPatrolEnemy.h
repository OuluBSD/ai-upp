#ifndef _RainbowGame_GroundPatrolEnemy_h_
#define _RainbowGame_GroundPatrolEnemy_h_

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
* Enemy with ground patrol AI that moves along the ground with occasional hops
*/
class GroundPatrolEnemy : public EnemyBase {
private:
    int direction; // 1 for right, -1 for left
    float patrolTimer; // Timer for when to hop or change behavior
    float patrolInterval; // How often to consider a behavior change
    bool isHopping;
    float hopTimer;
    float hopVelocityY; // Vertical velocity when hopping

public:
    GroundPatrolEnemy(float x, float y, float width, float height, 
                     const EnemyData& enemyData, const String& initialDirection);
    
    virtual void Update(float delta, Player* player, 
                       TileCollision* collision, 
                       EnemyProjectileManager* projectileManager) override;

private:
    /**
    * Handle the hopping behavior
    */
    void HandleHopping(float delta, TileCollision* collision);
    
    /**
    * Consider changing patrol behavior
    */
    void ConsiderPatrolChange();
};

#endif