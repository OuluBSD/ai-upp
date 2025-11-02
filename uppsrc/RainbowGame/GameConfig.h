#ifndef RAINBOWGAME_GAMECONFIG_H
#define RAINBOWGAME_GAMECONFIG_H

#include <Core/Core.h>

using namespace Upp;

class GameConfig {
public:
    GameConfig();
    ~GameConfig();
    
    // Game parameters
    float GetPlayerSpeed() const;
    float GetGravity() const;
    int GetMaxLives() const;
    int GetStartingCoins() const;
    
    // Level parameters
    int GetMaxLevels() const;
    int GetLevelsPerWorld() const;
    
    // Visual parameters
    float GetParticleEffectScale() const;
    
    // Update these getters with appropriate values
    void SetPlayerSpeed(float speed);
    void SetGravity(float gravity);
    void SetMaxLives(int lives);
    void SetStartingCoins(int coins);
    void SetMaxLevels(int levels);
    void SetLevelsPerWorld(int levels);
    void SetParticleEffectScale(float scale);
    
private:
    float playerSpeed;
    float gravity;
    int maxLives;
    int startingCoins;
    int maxLevels;
    int levelsPerWorld;
    float particleEffectScale;
};

#endif