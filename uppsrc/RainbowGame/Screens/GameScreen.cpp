#include "GameScreen.h"
#include "PauseScreen.h"
#include "GameOverScreen.h"
#include "MainMenuScreen.h"
#include "WorldSelectScreen.h"

GameScreen::GameScreen(RainbowGame* game, int world, int levelIndex) 
    : GameScreen(game, world, levelIndex, false, nullptr) {}

GameScreen::GameScreen(RainbowGame* game, int world, int levelIndex, bool editorMode, Screen* returnToScreen) 
    : game(game), world(world), levelIndex(levelIndex), editorMode(editorMode), 
      returnToScreen(returnToScreen), state(GameState::PLAYING), stateTimer(0.0f),
      player(nullptr), graphics(nullptr), mapWidthTiles(100), mapHeightTiles(100), 
      tileSize(14.0f), levelElapsedTime(0.0f), timeSinceLastPlayerHit(INFINITY), 
      levelDeathCount(0), recentHitCount(0), comboChainCount(0), animationStateTime(0.0f) {
    // Initialize the game screen
    LoadLevel();
    ConfigureCamera();
    RebuildAtlas();
    LoadPlayer1Entity();
    LoadWorldWallTexture();
    PlayWorldMusic();
}

GameScreen::~GameScreen() {
    if (player) {
        delete player;
        player = nullptr;
    }
}

void GameScreen::Show() {
    LOG("GameScreen: Show");
}

void GameScreen::Render(float deltaTime) {
    if (state == GameState::PLAYING) {
        UpdateGameplay(deltaTime);
    } else {
        stateTimer += deltaTime;
        if (state == GameState::LEVEL_COMPLETE && stateTimer > 2.0f) {
            // Go to next level - simplified implementation
            // In a real implementation, this would load the next level
        }
    }
    
    DrawScene();
}

void GameScreen::Hide() {
    LOG("GameScreen: Hide");
}

void GameScreen::UpdateGameplay(float delta) {
    InputState input = inputController.Update();
    
    if (input.pausePressed) {
        // In a full implementation, this would show the pause screen
        // game->SetScreen(new PauseScreen(game, this, [this]() { 
        //     game->SetScreen(new MainMenuScreen(game)); 
        // }));
        return;
    }
    
    levelElapsedTime += delta;
    if (isfinite(timeSinceLastPlayerHit)) {
        timeSinceLastPlayerHit += delta;
    }
    
    if (timeSinceLastPlayerHit >= 8.0f) {
        recentHitCount = 0;
        timeSinceLastPlayerHit = INFINITY;
    }
    
    // Update player with fixed time step (simplified)
    if (player) {
        // This is a simplified version - in a real implementation, you'd use a FixedTimeStepRunner
        player->Update(delta, input, nullptr); // Collision handler would be passed here
        HandlePlayerVerticalWrap();
        // In a real implementation, you would handle enemy updates as well
        HandleInteractions(delta);
    }
    
    if (input.attackPressed) {
        ThrowCapturedEnemies();
    }
    
    // Check win condition - simplified
    if (player && player->GetLives() <= 0) {
        state = GameState::GAME_OVER;
        stateTimer = 0.0f;
        // game->SetScreen(new GameOverScreen(game, world, levelIndex));
    }
}

void GameScreen::DrawScene() {
    // Clear screen with a background color
    if (!graphics) graphics = new Graphics();
    graphics->Clear(0.12f, 0.14f, 0.24f);  // Dark blue background like in pseudocode
    
    // Draw game elements
    DrawTiles();
    DrawPlayer();
    DrawEnemies();
    DrawPickups();
    
    // Present the frame
    graphics->Present();
}

void GameScreen::LoadLevel() {
    // Simplified level loading - in a real implementation, this would load actual level data
    LOG("Loading level: World " + AsString(world) + ", Level " + AsString(levelIndex));
    // In a real implementation, you would load level data and initialize collision map
}

void GameScreen::ConfigureCamera() {
    // Simplified camera configuration
    LOG("Configuring camera");
    // In a real implementation, you would set up an orthographic camera
}

void GameScreen::RebuildAtlas() {
    // Simplified texture atlas rebuilding
    LOG("Rebuilding texture atlas");
    // In a real implementation, you would load textures based on sprite scale
}

void GameScreen::LoadPlayer1Entity() {
    // Create a simple player at a starting position
    player = new Player(100.0f, 100.0f, 20.0f, 40.0f);
    LOG("Loaded player entity");
}

void GameScreen::LoadWorldWallTexture() {
    // Simplified wall texture loading
    LOG("Loading world wall texture");
}

void GameScreen::PlayWorldMusic() {
    // Play music for the current world - simplified
    if (game && game->GetAudioSystem()) {
        String track = "world" + AsString(world); // Simplified track name
        game->GetAudioSystem()->PlayMusic(track);
    }
}

void GameScreen::HandlePlayerVerticalWrap() {
    if (!player) return;
    
    Rectangle bounds = player->GetBounds();
    float mapHeightPixels = mapHeightTiles * tileSize;
    float topMarginPixels = 2 * tileSize; // 2 tiles for top margin
    float bottomMarginPixels = 2 * tileSize; // 2 tiles for bottom margin
    
    if (bounds.y <= -bottomMarginPixels + 0.001f) {
        float newY = mapHeightPixels + topMarginPixels - bounds.height - 0.001f;
        player->TeleportTo(bounds.x, newY);
    }
}

void GameScreen::HandleInteractions(float delta) {
    if (!player) return;
    
    // Simplified interaction handling - in a real implementation, this would check for 
    // collisions with enemies, pickups, goal, etc.
    Rectangle playerBounds = player->GetBounds();
    
    // Check for collisions with enemies (simplified)
    // In a full implementation, you would iterate through all enemies
    
    // Check for collisions with pickups (simplified)
    // In a full implementation, you would iterate through all pickups
    
    // Check for collision with goal (simplified)
    // In a full implementation, you would check if player overlaps with goal
}

void GameScreen::ThrowCapturedEnemies() {
    if (!player) return;
    
    // Simplified enemy throwing - in a real implementation, this would actually throw enemies
    LOG("Throwing captured enemies");
}

void GameScreen::UpdateThrownEnemies(float delta) {
    // In a real implementation, this would update the physics of thrown enemies
    // and check for collisions
}

void GameScreen::DrawPlayer() {
    if (!player) return;
    
    // In a real implementation, this would draw the actual player sprite
    // For now, we'll draw a simple rectangle
    SDL_SetRenderDrawColor(SdlWrapper::GetRenderer(), 0x26, 0x4b, 0x60, 0xFF); // COLOR_PLAYER from pseudocode
    
    Rectangle bounds = player->GetBounds();
    SDL_Rect rect = { (int)bounds.x, (int)bounds.y, (int)bounds.width, (int)bounds.height };
    SDL_RenderFillRect(SdlWrapper::GetRenderer(), &rect);
}

void GameScreen::DrawEnemies() {
    // In a real implementation, this would draw all enemies
    // For now, just a placeholder
    LOG("Drawing enemies");
}

void GameScreen::DrawPickups() {
    // In a real implementation, this would draw all pickups
    // For now, just a placeholder
    LOG("Drawing pickups");
}

void GameScreen::DrawTiles() {
    // Simplified tile drawing - in a real implementation, this would draw the actual level tiles
    // For now, just draw a simple grid pattern
    
    // Set color to represent a simple background (simplified)
    SDL_SetRenderDrawColor(SdlWrapper::GetRenderer(), 0x10, 0x18, 0x22, 0xFF); // Background color
    
    // Draw a simple pattern for demonstration
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            // Only draw every other tile to make a pattern
            if ((row + col) % 2 == 0) {
                SDL_Rect tileRect = { col * 40, row * 40, 40, 40 };
                SDL_RenderFillRect(SdlWrapper::GetRenderer(), &tileRect);
            }
        }
    }
}