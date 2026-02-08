#ifndef _Umbrella_GameScreen_h_
#define _Umbrella_GameScreen_h_

#include <CtrlLib/CtrlLib.h>
#include "LayerManager.h"
#include "Player.h"
#include "Enemy.h"

using namespace Upp;

// Forward declarations
class Treat;

enum GameState {
	PLAYING,
	PAUSED,
	GAME_OVER,
	LEVEL_COMPLETE,
	TRANSITION_HOVER,      // Player hovering with umbrella
	TRANSITION_SCROLL,     // Levels scrolling horizontally
	TRANSITION_DROP        // Player dropping into new level
};

class GameScreen : public TopWindow, public Player::CollisionHandler, public Player::CoordinateConverter {
public:  // Public for testing
	String levelPath;
	LayerManager layerManager;

	// Viewport/Camera
	Rect viewBounds;
	Point cameraOffset;
	float zoom;

	// Game loop timing
	int64 lastTime;
	static constexpr double FIXED_TIMESTEP = 1.0 / 60.0;
	double accumulator;

	// Game state
	GameState gameState;
	float levelCompleteTimer;  // Countdown for treat collection before transition
	bool allEnemiesKilled;  // Track if all enemies have been defeated

	// Transition state
	float transitionOffset;  // Horizontal scroll offset during transition
	String nextLevelPath;  // Path to next level to load

	// Level dimensions
	int levelColumns;
	int levelRows;
	int gridSize;

	// Player
	Player player;
	InputState inputState;

	// Enemies
	Array<Enemy*> enemies;

	// Treats (rewards from defeated enemies)
	Array<Treat*> treats;

	// Input tracking
	bool keyLeft, keyRight, keyJump, keyAttack;
	bool prevKeyJump, prevKeyAttack;

public:
	GameScreen();
	GameScreen(const String& level);

	// Main loop
	void GameTick(float delta);
	void LayoutLoop();

	// Rendering
	virtual void Paint(Draw& w) override;
	void RenderTiles(Draw& w);
	void RenderHUD(Draw& w);
	void RenderPauseScreen(Draw& w);
	void RenderGameOverScreen(Draw& w);
	void RenderLevelCompleteScreen(Draw& w);

	// Camera
	void UpdateCamera(Point targetPos);
	virtual Point WorldToScreen(Point worldPos) override;  // CoordinateConverter interface
	Point ScreenToWorld(Point screenPos);

	// Level loading
	bool LoadLevel(const String& path);
	String GetNextLevelPath(const String& currentPath);

	// Player spawning
	Point FindSpawnPoint();
	void RespawnPlayer();

	// Enemy management
	void SpawnEnemies();
	void ClearEnemies();

	// Input
	virtual bool Key(dword key, int) override;
	void UpdateInput();

	// Game state
	void SetGameState(GameState newState);
	GameState GetGameState() const { return gameState; }
	void HandleGameOver();
	void RestartLevel();

	// CollisionHandler interface
	virtual bool IsFullBlockTile(int col, int row) override;
	virtual bool IsWallTile(int col, int row) override;
	virtual bool IsFloorTile(int col, int row) override;
	virtual float GetGridSize() override { return (float)gridSize; }
};

#endif
