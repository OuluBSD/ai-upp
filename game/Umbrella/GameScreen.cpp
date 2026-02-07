#include "Umbrella.h"
#include "GameScreen.h"
#include "MapSerializer.h"
#include "EnemyPatroller.h"
#include "EnemyJumper.h"
#include "EnemyShooter.h"

using namespace Upp;

GameScreen::GameScreen() : player(100, 100, 12, 12) {
	Title("Umbrella - Game");
	Sizeable().Zoomable();
	SetRect(0, 0, 1280, 720);

	zoom = 2.0f;
	cameraOffset = Point(0, 0);
	accumulator = 0.0;
	gameState = PLAYING;
	levelColumns = 0;
	levelRows = 0;
	gridSize = 14;

	// Input state
	keyLeft = keyRight = keyJump = keyAttack = false;
	prevKeyJump = prevKeyAttack = false;

	lastTime = GetTickCount();

	// Start game loop
	SetTimeCallback(16, [=] { LayoutLoop(); });
}

GameScreen::GameScreen(const String& level) : GameScreen() {
	levelPath = level;
	LoadLevel(level);
}

bool GameScreen::LoadLevel(const String& path) {
	if(!MapSerializer::LoadFromFile(path, layerManager)) {
		Exclamation("Failed to load: " + path);
		return false;
	}

	// Get level dimensions from terrain layer
	Layer* terrainLayer = layerManager.FindLayerByType(LAYER_TERRAIN);
	if(terrainLayer) {
		const MapGrid& grid = terrainLayer->GetGrid();
		levelColumns = grid.GetColumns();
		levelRows = grid.GetRows();
		gridSize = 14;  // Default grid size
	}

	// Spawn player at appropriate location
	RespawnPlayer();

	// Spawn enemies
	SpawnEnemies();

	Title("Umbrella - " + GetFileName(path));
	return true;
}

Point GameScreen::FindSpawnPoint() {
	// Find first solid ground starting from top-left of map
	// Search column by column, top to bottom
	for(int col = 1; col < levelColumns - 1; col++) {
		// Start from near top of map and search downward (remember Y-up: high Y = top)
		for(int row = levelRows - 3; row >= 1; row--) {
			// Check if there's solid ground at this position
			if(IsFloorTile(col, row)) {
				// Found ground - spawn player one tile above it
				int spawnX = col * gridSize + gridSize / 2 - 6;  // Center of tile, adjusted for player width
				int spawnY = (row + 1) * gridSize;  // One tile above ground
				return Point(spawnX, spawnY);
			}
		}
	}

	// Fallback: spawn near top-center of map
	int spawnX = (levelColumns / 2) * gridSize;
	int spawnY = (levelRows - 5) * gridSize;
	return Point(spawnX, spawnY);
}

void GameScreen::RespawnPlayer() {
	Point spawn = FindSpawnPoint();
	player.SetPosition((float)spawn.x, (float)spawn.y);
}

void GameScreen::LayoutLoop() {
	// Only update game logic if playing
	if(gameState == PLAYING) {
		int64 now = GetTickCount();
		double delta = (now - lastTime) / 1000.0;
		lastTime = now;

		// Cap delta to prevent spiral of death
		if(delta > 0.25) {
			delta = 0.25;
		}

		accumulator += delta;

		// Fixed timestep update (60 FPS physics)
		while(accumulator >= FIXED_TIMESTEP) {
			GameTick((float)FIXED_TIMESTEP);
			accumulator -= FIXED_TIMESTEP;
		}
	}

	Refresh();  // Always render (shows pause/game over screens)
	SetTimeCallback(16, [=] { LayoutLoop(); });  // ~60 FPS render
}

void GameScreen::GameTick(float delta) {
	// Update input state
	UpdateInput();

	// Update player
	player.Update(delta, inputState, *this);

	// Check if player fell off map (below Y=0)
	Pointf playerPos = player.GetPosition();
	if(playerPos.y < -gridSize * 2) {
		// Player fell off - respawn and take damage
		player.TakeDamage(1);
		RespawnPlayer();

		// Check for game over
		if(player.GetLives() <= 0) {
			HandleGameOver();
		}
	}

	// Update camera to follow player
	Pointf playerCenter = player.GetCenter();
	UpdateCamera(Point((int)playerCenter.x, (int)playerCenter.y));

	// Update enemies
	for(int i = 0; i < enemies.GetCount(); i++) {
		enemies[i]->Update(delta, player, *this);
	}

	// Check player-enemy collisions
	Rectf playerBounds = player.GetBounds();
	for(int i = 0; i < enemies.GetCount(); i++) {
		if(!enemies[i]->IsAlive()) continue;

		Rectf enemyBounds = enemies[i]->GetBounds();

		// Check if bounds intersect
		if(playerBounds.left < enemyBounds.right &&
		   playerBounds.right > enemyBounds.left &&
		   min(playerBounds.top, playerBounds.bottom) < max(enemyBounds.top, enemyBounds.bottom) &&
		   max(playerBounds.top, playerBounds.bottom) > min(enemyBounds.top, enemyBounds.bottom)) {
			// Collision detected - player takes damage
			player.TakeDamage(1);
			// Knockback could be added here
		}

		// Check projectile collisions (for shooters)
		if(enemies[i]->GetType() == ENEMY_SHOOTER) {
			EnemyShooter* shooter = dynamic_cast<EnemyShooter*>(enemies[i]);
			if(shooter) {
				const Array<Projectile*>& projectiles = shooter->GetProjectiles();
				for(int j = 0; j < projectiles.GetCount(); j++) {
					if(!projectiles[j]->IsActive()) continue;

					Rectf projBounds = projectiles[j]->GetBounds();

					// Check if projectile hits player
					if(playerBounds.left < projBounds.right &&
					   playerBounds.right > projBounds.left &&
					   min(playerBounds.top, playerBounds.bottom) < max(projBounds.top, projBounds.bottom) &&
					   max(playerBounds.top, playerBounds.bottom) > min(projBounds.top, projBounds.bottom)) {
						// Projectile hit player
						player.TakeDamage(1);
						projectiles[j]->Deactivate();
					}
				}
			}
		}
	}
}

void GameScreen::UpdateCamera(Point targetPos) {
	Size screenSize = GetSize();

	// Calculate camera bounds (center on target)
	int viewWidth = screenSize.cx / zoom;
	int viewHeight = screenSize.cy / zoom;

	cameraOffset.x = targetPos.x - viewWidth / 2;
	cameraOffset.y = targetPos.y - viewHeight / 2;

	// Clamp to level bounds
	int levelWidth = levelColumns * gridSize;
	int levelHeight = levelRows * gridSize;

	cameraOffset.x = max(0, min(cameraOffset.x, levelWidth - viewWidth));
	cameraOffset.y = max(0, min(cameraOffset.y, levelHeight - viewHeight));
}

Point GameScreen::WorldToScreen(Point worldPos) {
	// World coordinates: Y-up (like OpenGL, standard game coordinates)
	// Screen coordinates: Y-down (U++ Draw/CtrlCore convention)
	// Flip Y at the last moment when converting to screen space
	Size sz = GetSize();
	int viewHeight = sz.cy / zoom;

	Point screenPos;
	screenPos.x = (worldPos.x - cameraOffset.x) * zoom;
	// Flip Y: worldY=0 is bottom, screenY=0 is top
	screenPos.y = sz.cy - ((worldPos.y - cameraOffset.y) * zoom);
	return screenPos;
}

Point GameScreen::ScreenToWorld(Point screenPos) {
	// Inverse of WorldToScreen
	Size sz = GetSize();
	int viewHeight = sz.cy / zoom;

	Point worldPos;
	worldPos.x = screenPos.x / zoom + cameraOffset.x;
	// Unflip Y
	worldPos.y = (sz.cy - screenPos.y) / zoom + cameraOffset.y;
	return worldPos;
}

void GameScreen::Paint(Draw& w) {
	Size sz = GetSize();

	// Clear background (dark blue)
	w.DrawRect(sz, Color(12, 17, 30));

	// Render tiles
	RenderTiles(w);

	// Render enemies
	for(int i = 0; i < enemies.GetCount(); i++) {
		enemies[i]->Render(w, *this);
	}

	// Render player (using WorldToScreen for proper Y-flip)
	player.Render(w, *this);

	// Render HUD (lives, score)
	RenderHUD(w);

	// Render overlays based on game state
	switch(gameState) {
		case PAUSED:
			RenderPauseScreen(w);
			break;
		case GAME_OVER:
			RenderGameOverScreen(w);
			break;
		case LEVEL_COMPLETE:
			RenderLevelCompleteScreen(w);
			break;
		default:
			break;
	}
}

void GameScreen::RenderTiles(Draw& w) {
	Size sz = GetSize();

	// Calculate visible tile range
	int tileSize = int(gridSize * zoom);
	if(tileSize < 1) tileSize = 1;

	int viewCols = sz.cx / tileSize + 2;
	int viewRows = sz.cy / tileSize + 2;

	int startCol = max(0, int(cameraOffset.x / gridSize));
	int startRow = max(0, int(cameraOffset.y / gridSize));
	int endCol = min(levelColumns, startCol + viewCols);
	int endRow = min(levelRows, startRow + viewRows);

	// Render each visible layer (bottom to top)
	// Render in REVERSE order so Annotations renders first (behind)
	for(int layerIndex = layerManager.GetLayerCount() - 1; layerIndex >= 0; layerIndex--) {
		const Layer& layer = layerManager.GetLayer(layerIndex);

		if(!layer.IsVisible()) continue;

		const MapGrid& grid = layer.GetGrid();
		int opacity = layer.GetOpacity();

		// Render tiles in this layer
		for(int row = startRow; row < endRow; row++) {
			for(int col = startCol; col < endCol; col++) {
				TileType tile = grid.GetTile(col, row);

				if(tile == TILE_EMPTY) continue;

				// Calculate screen position (world to screen with camera)
				// World coordinates: (col*gridSize, row*gridSize) is bottom-left of tile
				Point worldBottomLeft(col * gridSize, row * gridSize);
				Point worldTopRight((col + 1) * gridSize, (row + 1) * gridSize);

				Point screenBottomLeft = WorldToScreen(worldBottomLeft);
				Point screenTopRight = WorldToScreen(worldTopRight);

				// After Y-flip, screenTopRight.y < screenBottomLeft.y
				// Normalize to get proper screen rect
				int screenX = min(screenBottomLeft.x, screenTopRight.x);
				int screenY = min(screenBottomLeft.y, screenTopRight.y);
				int width = abs(screenTopRight.x - screenBottomLeft.x);
				int height = abs(screenTopRight.y - screenBottomLeft.y);

				// Get tile color
				Color tileColor = TileTypeToColor(tile);

				// Apply layer opacity
				if(opacity < 100) {
					Color bgColor = Color(12, 17, 30);
					int alpha = opacity * 255 / 100;
					tileColor = Color(
						(tileColor.GetR() * alpha + bgColor.GetR() * (255 - alpha)) / 255,
						(tileColor.GetG() * alpha + bgColor.GetG() * (255 - alpha)) / 255,
						(tileColor.GetB() * alpha + bgColor.GetB() * (255 - alpha)) / 255
					);
				}

				// Draw tile as filled rectangle
				w.DrawRect(screenX, screenY, width, height, tileColor);
			}
		}
	}
}

bool GameScreen::Key(dword key, int) {
	// Handle key downs based on game state
	switch(key) {
		case K_ESCAPE:
			if(gameState == PLAYING) {
				SetGameState(PAUSED);
				return true;
			} else if(gameState == PAUSED) {
				SetGameState(PLAYING);
				return true;
			}
			Close();
			return true;

		case K_P:
			if(gameState == PLAYING) {
				SetGameState(PAUSED);
			} else if(gameState == PAUSED) {
				SetGameState(PLAYING);
			}
			return true;

		case K_R:
			// Restart on game over or level complete
			if(gameState == GAME_OVER || gameState == LEVEL_COMPLETE) {
				RestartLevel();
				return true;
			}
			break;

		// Movement keys (only work when playing)
		case K_LEFT:
		case K_A:
			if(gameState == PLAYING) keyLeft = true;
			return true;
		case K_RIGHT:
		case K_D:
			if(gameState == PLAYING) keyRight = true;
			return true;

		// Jump keys (only work when playing)
		case K_SPACE:
		case K_W:
		case K_UP:
			if(gameState == PLAYING) keyJump = true;
			return true;

		// Attack/Glide (only work when playing)
		case K_CTRL_RIGHT:
			if(gameState == PLAYING) keyAttack = true;
			return true;

		// Key ups (use bit flag to detect release)
		case K_LEFT | K_KEYUP:
		case K_A | K_KEYUP:
			keyLeft = false;
			return true;
		case K_RIGHT | K_KEYUP:
		case K_D | K_KEYUP:
			keyRight = false;
			return true;
		case K_SPACE | K_KEYUP:
		case K_W | K_KEYUP:
		case K_UP | K_KEYUP:
			keyJump = false;
			return true;
		case K_CTRL_RIGHT | K_KEYUP:
			keyAttack = false;
			return true;

		default:
			break;
	}
	return false;
}

void GameScreen::UpdateInput() {
	// Build input state from tracked keys
	inputState.moveLeft = keyLeft;
	inputState.moveRight = keyRight;
	inputState.jumpHeld = keyJump;
	inputState.jumpPressed = keyJump && !prevKeyJump;
	inputState.attackPressed = keyAttack && !prevKeyAttack;
	inputState.glideHeld = keyAttack;

	prevKeyJump = keyJump;
	prevKeyAttack = keyAttack;
}

bool GameScreen::IsFullBlockTile(int col, int row) {
	Layer* terrainLayer = layerManager.FindLayerByType(LAYER_TERRAIN);
	if(!terrainLayer) return false;

	return terrainLayer->GetGrid().GetTile(col, row) == TILE_FULLBLOCK;
}

bool GameScreen::IsWallTile(int col, int row) {
	Layer* terrainLayer = layerManager.FindLayerByType(LAYER_TERRAIN);
	if(!terrainLayer) return false;

	return terrainLayer->GetGrid().GetTile(col, row) == TILE_WALL;
}

bool GameScreen::IsFloorTile(int col, int row) {
	return IsWallTile(col, row) || IsFullBlockTile(col, row);
}

void GameScreen::RenderHUD(Draw& w) {
	Size sz = GetSize();

	// Draw dark background bar at top
	Color bgColor = Color(0, 0, 0);
	w.DrawRect(0, 0, sz.cx, 40, bgColor);

	// Draw lives (hearts)
	String livesText = Format("Lives: %d", player.GetLives());
	Font fnt = Arial(20).Bold();
	w.DrawText(20, 10, livesText, fnt, White());

	// Draw heart symbols
	Color heartColor = Color(255, 50, 50);
	for(int i = 0; i < player.GetLives(); i++) {
		int x = 120 + i * 25;
		int y = 10;
		// Draw simple heart shape as two rects (placeholder)
		w.DrawRect(x, y + 5, 8, 8, heartColor);
		w.DrawRect(x + 8, y + 5, 8, 8, heartColor);
		w.DrawRect(x + 2, y + 13, 12, 8, heartColor);
	}

	// Draw score
	String scoreText = Format("Score: %d", player.GetScore());
	Size textSz = GetTextSize(scoreText, fnt);
	w.DrawText(sz.cx - textSz.cx - 20, 10, scoreText, fnt, White());
}

void GameScreen::RenderPauseScreen(Draw& w) {
	Size sz = GetSize();

	// Semi-transparent overlay
	Color overlayColor = Color(0, 0, 0);
	w.DrawRect(0, 0, sz.cx, sz.cy, overlayColor);

	// Title
	Font titleFont = Arial(48).Bold();
	String pauseText = "PAUSED";
	Size titleSz = GetTextSize(pauseText, titleFont);
	w.DrawText((sz.cx - titleSz.cx) / 2, sz.cy / 2 - 60, pauseText, titleFont, White());

	// Instructions
	Font instructFont = Arial(24);
	String resumeText = "Press ESC or P to Resume";
	Size resumeSz = GetTextSize(resumeText, instructFont);
	w.DrawText((sz.cx - resumeSz.cx) / 2, sz.cy / 2 + 20, resumeText, instructFont, White());
}

void GameScreen::RenderGameOverScreen(Draw& w) {
	Size sz = GetSize();

	// Dark overlay
	Color overlayColor = Color(20, 0, 0);
	w.DrawRect(0, 0, sz.cx, sz.cy, overlayColor);

	// Title
	Font titleFont = Arial(56).Bold();
	String gameOverText = "GAME OVER";
	Size titleSz = GetTextSize(gameOverText, titleFont);
	w.DrawText((sz.cx - titleSz.cx) / 2, sz.cy / 2 - 80, gameOverText, titleFont, Color(255, 50, 50));

	// Score
	Font scoreFont = Arial(32);
	String finalScore = Format("Final Score: %d", player.GetScore());
	Size scoreSz = GetTextSize(finalScore, scoreFont);
	w.DrawText((sz.cx - scoreSz.cx) / 2, sz.cy / 2, finalScore, scoreFont, White());

	// Instructions
	Font instructFont = Arial(24);
	String restartText = "Press R to Restart";
	Size restartSz = GetTextSize(restartText, instructFont);
	w.DrawText((sz.cx - restartSz.cx) / 2, sz.cy / 2 + 60, restartText, instructFont, Color(200, 200, 200));

	String quitText = "Press ESC to Quit";
	Size quitSz = GetTextSize(quitText, instructFont);
	w.DrawText((sz.cx - quitSz.cx) / 2, sz.cy / 2 + 100, quitText, instructFont, Color(200, 200, 200));
}

void GameScreen::RenderLevelCompleteScreen(Draw& w) {
	Size sz = GetSize();

	// Light overlay
	Color overlayColor = Color(0, 20, 0);
	w.DrawRect(0, 0, sz.cx, sz.cy, overlayColor);

	// Title
	Font titleFont = Arial(56).Bold();
	String completeText = "LEVEL COMPLETE!";
	Size titleSz = GetTextSize(completeText, titleFont);
	w.DrawText((sz.cx - titleSz.cx) / 2, sz.cy / 2 - 80, completeText, titleFont, Color(50, 255, 50));

	// Score
	Font scoreFont = Arial(32);
	String finalScore = Format("Score: %d", player.GetScore());
	Size scoreSz = GetTextSize(finalScore, scoreFont);
	w.DrawText((sz.cx - scoreSz.cx) / 2, sz.cy / 2, finalScore, scoreFont, White());

	// Instructions
	Font instructFont = Arial(24);
	String continueText = "Press R to Continue";
	Size continueSz = GetTextSize(continueText, instructFont);
	w.DrawText((sz.cx - continueSz.cx) / 2, sz.cy / 2 + 60, continueText, instructFont, Color(200, 200, 200));
}

void GameScreen::SetGameState(GameState newState) {
	gameState = newState;

	// Reset input state when changing states
	keyLeft = keyRight = keyJump = keyAttack = false;
	prevKeyJump = prevKeyAttack = false;
}

void GameScreen::HandleGameOver() {
	SetGameState(GAME_OVER);
}

void GameScreen::RestartLevel() {
	// Reload the level
	if(!levelPath.IsEmpty()) {
		LoadLevel(levelPath);
		SetGameState(PLAYING);
		lastTime = GetTickCount();
		accumulator = 0.0;
	}
}

void GameScreen::ClearEnemies() {
	for(int i = 0; i < enemies.GetCount(); i++) {
		delete enemies[i];
	}
	enemies.Clear();
}

void GameScreen::SpawnEnemies() {
	ClearEnemies();

	// Spawn mixed enemy types at various positions
	// For now, spawn enemies manually at reasonable positions
	// Later this could be data-driven from map files

	if(levelColumns > 10 && levelRows > 5) {
		// Spawn patroller at column 8, looking for ground
		for(int row = levelRows - 3; row >= 1; row--) {
			if(IsFloorTile(8, row)) {
				int spawnX = 8 * gridSize;
				int spawnY = (row + 1) * gridSize;
				enemies.Add(new EnemyPatroller((float)spawnX, (float)spawnY));
				break;
			}
		}

		// Spawn jumper at column 15
		for(int row = levelRows - 3; row >= 1; row--) {
			if(IsFloorTile(15, row)) {
				int spawnX = 15 * gridSize;
				int spawnY = (row + 1) * gridSize;
				enemies.Add(new EnemyJumper((float)spawnX, (float)spawnY));
				break;
			}
		}

		// Spawn patroller at column 22
		for(int row = levelRows - 3; row >= 1; row--) {
			if(IsFloorTile(22, row)) {
				int spawnX = 22 * gridSize;
				int spawnY = (row + 1) * gridSize;
				enemies.Add(new EnemyPatroller((float)spawnX, (float)spawnY));
				break;
			}
		}

		// Spawn jumper at column 30
		if(levelColumns > 30) {
			for(int row = levelRows - 3; row >= 1; row--) {
				if(IsFloorTile(30, row)) {
					int spawnX = 30 * gridSize;
					int spawnY = (row + 1) * gridSize;
					enemies.Add(new EnemyJumper((float)spawnX, (float)spawnY));
					break;
				}
			}
		}

		// Spawn shooter at column 12
		for(int row = levelRows - 3; row >= 1; row--) {
			if(IsFloorTile(12, row)) {
				int spawnX = 12 * gridSize;
				int spawnY = (row + 1) * gridSize;
				enemies.Add(new EnemyShooter((float)spawnX, (float)spawnY));
				break;
			}
		}

		// Spawn shooter at column 25
		if(levelColumns > 25) {
			for(int row = levelRows - 3; row >= 1; row--) {
				if(IsFloorTile(25, row)) {
					int spawnX = 25 * gridSize;
					int spawnY = (row + 1) * gridSize;
					enemies.Add(new EnemyShooter((float)spawnX, (float)spawnY));
					break;
				}
			}
		}
	}
}
