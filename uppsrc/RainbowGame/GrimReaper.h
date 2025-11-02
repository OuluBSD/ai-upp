#ifndef _RainbowGame_GrimReaper_h_
#define _RainbowGame_GrimReaper_h_

#include <Core/Core.h>
#include "EnemyBase.h"
#include "Player.h"
#include "EnemyData.h"
#include "EnemyAudioManager.h"
#include "TileCollision.h"
#include "EnemyProjectileManager.h"

using namespace Upp;

// Forward declarations
class Player;
class TileCollision;
class EnemyProjectileManager;
class EnemyAudioManager;

/**
* Implementation of the Grim Reaper which appears as a fail state
* The Grim Reaper follows the player and instantly kills them on contact
*/
class GrimReaper : public EnemyBase {
private:
    static constexpr float GRIM_REAPER_SPEED = 80.0f; // Slow but relentless
    static constexpr float GRIM_REAPER_ZIGZAG_AMPLITUDE = 20.0f; // Horizontal movement amplitude
    static constexpr float GRIM_REAPER_ZIGZAG_FREQUENCY = 2.0f; // How often it zigzags
    float zigzagTimer;
    bool isChasing;
    bool musicSwapped;

public:
    GrimReaper(float x, float y);
    
    /**
    * Activate the chasing behavior and swap music
    */
    void ActivateChase();
    
    /**
    * Swap music to the Grim Reaper theme
    */
    void SwapMusic(EnemyAudioManager* audioManager);
    
    virtual void Update(float delta, Player* player, 
                       TileCollision* collision, 
                       EnemyProjectileManager* projectileManager) override;
    
    /**
    * Check if Grim Reaper has caught the player
    */
    bool HasCaughtPlayer(Player* player);
    
    bool IsChasing() const { return isChasing; }

private:
    EnemyData CreateGrimReaperData();
};

#endif