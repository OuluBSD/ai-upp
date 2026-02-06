#ifndef _Umbrella_GameScreen_h_
#define _Umbrella_GameScreen_h_

#include <CtrlLib/CtrlLib.h>
#include "LayerManager.h"
#include "Player.h"
#include "Enemy.h"

using namespace Upp;

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
	bool paused;

	// Level dimensions
	int levelColumns;
	int levelRows;
	int gridSize;

	// Player
	Player player;
	InputState inputState;

	// Enemies
	Array<Enemy*> enemies;

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

	// Camera
	void UpdateCamera(Point targetPos);
	virtual Point WorldToScreen(Point worldPos) override;  // CoordinateConverter interface
	Point ScreenToWorld(Point screenPos);

	// Level loading
	bool LoadLevel(const String& path);

	// Player spawning
	Point FindSpawnPoint();
	void RespawnPlayer();

	// Enemy management
	void SpawnEnemies();
	void ClearEnemies();

	// Input
	virtual bool Key(dword key, int) override;
	void UpdateInput();

	// CollisionHandler interface
	virtual bool IsFullBlockTile(int col, int row) override;
	virtual bool IsWallTile(int col, int row) override;
	virtual bool IsFloorTile(int col, int row) override;
	virtual float GetGridSize() override { return (float)gridSize; }
};

#endif
