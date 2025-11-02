#include "GameConfig.h"

GameConfig::GameConfig() 
    : playerSpeed(200.0f), gravity(400.0f), maxLives(3), 
      startingCoins(0), maxLevels(30), levelsPerWorld(10),
      particleEffectScale(1.0f) {
    // Initialize with default game configuration values
}

GameConfig::~GameConfig() {
    // Cleanup
}

float GameConfig::GetPlayerSpeed() const {
    return playerSpeed;
}

float GameConfig::GetGravity() const {
    return gravity;
}

int GameConfig::GetMaxLives() const {
    return maxLives;
}

int GameConfig::GetStartingCoins() const {
    return startingCoins;
}

int GameConfig::GetMaxLevels() const {
    return maxLevels;
}

int GameConfig::GetLevelsPerWorld() const {
    return levelsPerWorld;
}

float GameConfig::GetParticleEffectScale() const {
    return particleEffectScale;
}

void GameConfig::SetPlayerSpeed(float speed) {
    playerSpeed = speed;
}

void GameConfig::SetGravity(float gravity) {
    this->gravity = gravity;
}

void GameConfig::SetMaxLives(int lives) {
    maxLives = lives;
}

void GameConfig::SetStartingCoins(int coins) {
    startingCoins = coins;
}

void GameConfig::SetMaxLevels(int levels) {
    maxLevels = levels;
}

void GameConfig::SetLevelsPerWorld(int levels) {
    levelsPerWorld = levels;
}

void GameConfig::SetParticleEffectScale(float scale) {
    particleEffectScale = scale;
}