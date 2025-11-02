#ifndef _RainbowGame_AirbornePathEnemy_h_
#define _RainbowGame_AirbornePathEnemy_h_

#include "EnemyBase.h"  // Assuming this is the base class
#include "EnemyData.h"
#include "Player.h"
#include "TileCollision.h"
#include "EnemyProjectileManager.h"

using namespace Upp;

// Forward declarations
class Player;
class TileCollision;
class EnemyProjectileManager;

// Enum for motion path types
enum class MotionPath {
    HORIZONTAL,  // Moves left and right
    VERTICAL,    // Moves up and down
    DIAGONAL     // Moves diagonally
};

class AirbornePathEnemy : public EnemyBase {
private:
    MotionPath motionPath;
    int horizontalDirection;  // -1 for left, 1 for right
    int verticalDirection;    // -1 for down, 1 for up
    float pathTimer;
    float pathChangeInterval;

public:
    AirbornePathEnemy(float x, float y, float width, float height,
                      const EnemyData& enemyData, MotionPath path);
    
    virtual void Update(float delta, Player* player,
                       TileCollision* collision,
                       EnemyProjectileManager* projectileManager) override;
    
private:
    void ConsiderPathChange();
};

#endif