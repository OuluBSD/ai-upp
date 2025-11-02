#ifndef RAINBOWGAME_SCREENS_GAMESCREEN_H
#define RAINBOWGAME_SCREENS_GAMESCREEN_H

#include <Core/Core.h>
#include <Gdx/Gdx.h>
#include "RainbowGame.h"
#include "Input/InputController.h"
#include "Gameplay/Components/Player.h"

using namespace Upp;

class CollisionHandler {
public:
    virtual ~CollisionHandler() {}
    virtual bool IsFullBlockTile(int col, int row) = 0;
    virtual bool IsWallTile(int col, int row) = 0;
    virtual bool IsFloorTile(int col, int row) = 0;
    virtual int GetColumns() = 0;
    virtual int GetRows() = 0;
};

class GameScreen : public Screen {
public:
    GameScreen(RainbowGame* game, int world, int levelIndex);
    GameScreen(RainbowGame* game, int world, int levelIndex, bool editorMode, Screen* returnToScreen);
    virtual ~GameScreen();
    
    virtual void Show() override;
    virtual void Render(float deltaTime) override;
    virtual void Hide() override;
    
private:
    enum class GameState {
        PLAYING,
        LEVEL_COMPLETE,
        GAME_OVER
    };
    
    void UpdateGameplay(float delta);
    void DrawScene();
    void LoadLevel();
    void ConfigureCamera();
    void RebuildAtlas();
    void LoadPlayer1Entity();
    void LoadWorldWallTexture();
    void PlayWorldMusic();
    void HandlePlayerVerticalWrap();
    void HandleInteractions(float delta);
    void ThrowCapturedEnemies();
    void UpdateThrownEnemies(float delta);
    void DrawPlayer();
    void DrawEnemies();
    void DrawPickups();
    void DrawTiles();
    
    RainbowGame* game;
    int world;
    int levelIndex;
    bool editorMode;
    Screen* returnToScreen;
    
    InputController inputController;
    Player* player;
    GameState state;
    float stateTimer;
    
    // Camera and rendering
    Graphics* graphics;
    
    // Level data would go here in a full implementation
    int mapWidthTiles;
    int mapHeightTiles;
    float tileSize;
    
    // Timing
    float levelElapsedTime;
    float timeSinceLastPlayerHit;
    int levelDeathCount;
    int recentHitCount;
    int comboChainCount;
    
    // Animation
    float animationStateTime;
};

#endif