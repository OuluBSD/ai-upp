#include "Umbrella.h"
#include "GameScreen.h"
#include "MapSerializer.h"

using namespace Upp;

GameScreen::GameScreen() : player(100, 100, 12, 12) {
	Title("Umbrella - Game");
	Sizeable().Zoomable();
	SetRect(0, 0, 1280, 720);

	zoom = 2.0f;
	cameraOffset = Point(0, 0);
	accumulator = 0.0;
	paused = false;
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
		Exclamation("Failed to load level: " + path);
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

	Title("Umbrella - " + GetFileName(path));
	return true;
}

void GameScreen::LayoutLoop() {
	if(paused) {
		SetTimeCallback(16, [=] { LayoutLoop(); });
		return;
	}

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

	Refresh();  // Trigger Paint()
	SetTimeCallback(16, [=] { LayoutLoop(); });  // ~60 FPS render
}

void GameScreen::GameTick(float delta) {
	// Update input state
	UpdateInput();

	// Update player
	player.Update(delta, inputState, *this);

	// Update camera to follow player
	Pointf playerCenter = player.GetCenter();
	UpdateCamera(Point((int)playerCenter.x, (int)playerCenter.y));

	// TODO: Update enemies
	// TODO: Check collisions
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
	Point screenPos;
	screenPos.x = (worldPos.x - cameraOffset.x) * zoom;
	screenPos.y = (worldPos.y - cameraOffset.y) * zoom;
	return screenPos;
}

Point GameScreen::ScreenToWorld(Point screenPos) {
	Point worldPos;
	worldPos.x = screenPos.x / zoom + cameraOffset.x;
	worldPos.y = screenPos.y / zoom + cameraOffset.y;
	return worldPos;
}

void GameScreen::Paint(Draw& w) {
	Size sz = GetSize();

	// Clear background (dark blue)
	w.DrawRect(sz, Color(12, 17, 30));

	// Render tiles
	RenderTiles(w);

	// Render player
	player.Render(w, cameraOffset, zoom);

	// TODO: Render enemies
	// TODO: Render HUD
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
				Point worldPos(col * gridSize, row * gridSize);
				Point screenPos = WorldToScreen(worldPos);

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
				w.DrawRect(screenPos.x, screenPos.y, tileSize, tileSize, tileColor);
			}
		}
	}
}

bool GameScreen::Key(dword key, int) {
	// Handle key downs
	switch(key) {
		case K_ESCAPE:
			Close();
			return true;
		case K_P:
			paused = !paused;
			return true;

		// Movement keys
		case K_LEFT:
		case K_A:
			keyLeft = true;
			return true;
		case K_RIGHT:
		case K_D:
			keyRight = true;
			return true;

		// Jump keys
		case K_SPACE:
		case K_W:
		case K_UP:
			keyJump = true;
			return true;

		// Attack/Glide
		case K_CTRL_RIGHT:
			keyAttack = true;
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
