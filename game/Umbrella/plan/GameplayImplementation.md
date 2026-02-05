# Umbrella Game - Gameplay Implementation Plan

**Status**: Planning
**Date**: 2026-02-05
**Based on**: RainbowGame Java source (GameScreen.java, Player.java, Enemy components)

---

## Overview

This track covers the conversion of RainbowGame's core gameplay systems from libGDX/Java to U++. The goal is to recreate the platformer mechanics, enemy AI, physics, and game loop in native U++ code.

---

## Architecture

### Java (libGDX) Architecture
```
RainbowGame (Game)
  └── GameScreen (Screen, LiveLevelListener)
       ├── SpriteBatch (rendering)
       ├── OrthographicCamera (viewport)
       ├── OrthogonalTiledMapRenderer (tile map)
       ├── Player (physics entity)
       ├── Array<Enemy> (AI entities)
       ├── DropletManager (collectibles)
       ├── HUD (UI overlay)
       └── FixedTimeStepRunner (game loop)
```

### U++ Target Architecture
```
UmbrellaGame (TopWindow)
  └── GameScreen (Ctrl/TopWindow)
       ├── ImageBuffer (render target)
       ├── Rect (viewport/camera)
       ├── TileRenderer (map rendering)
       ├── Player (physics entity)
       ├── Array<Enemy*> (AI entities)
       ├── CollectibleManager (items)
       ├── GameHUD (ParentCtrl overlay)
       └── TimeCallback (game loop)
```

---

## Phase 1: Core Game Loop and Rendering

### Task 1.1: GameScreen Foundation
**File**: `game/Umbrella/GameScreen.h`, `GameScreen.cpp`
**Dependencies**: MapSerializer, LevelLoader

**Implementation**:
1. Create `GameScreen` class inheriting from `TopWindow`
2. Add level loading via `MapSerializer::LoadFromFile()`
3. Implement basic tile rendering using existing `LayerManager`
4. Set up viewport/camera system (Rect for view bounds)
5. Add fixed timestep game loop using `SetTimeCallback()`

**Reference**: `GameScreen.java:61-100` (constructor and initialization)

**API Design**:
```cpp
class GameScreen : public TopWindow {
private:
    String levelPath;
    LayerManager layerManager;
    Rect viewBounds;
    Point cameraOffset;
    float zoom;

    // Game loop
    int64 lastTime;
    const double FIXED_TIMESTEP = 1.0 / 60.0;
    double accumulator;

public:
    GameScreen(const String& level);
    void GameTick(float delta);      // Fixed update
    void Paint(Draw& w) override;    // Render
    void RenderTiles(Draw& w);
};
```

**Success Criteria**:
- Loads level JSON correctly
- Renders tile layers (walls, background, blocks)
- Camera follows viewport bounds
- 60 FPS fixed timestep game loop running

---

### Task 1.2: Camera System
**File**: `GameScreen.cpp` (camera methods)
**Dependencies**: Player position

**Implementation**:
1. Implement `UpdateCamera()` method
2. Camera follows player with smoothing
3. Clamp camera to level bounds
4. Convert world coords to screen coords

**Reference**: `CameraSystem.java` (camera tracking logic)

**API Design**:
```cpp
void UpdateCamera(Point playerPos, Size levelSize);
Point WorldToScreen(Point worldPos);
Point ScreenToWorld(Point screenPos);
```

---

## Phase 2: Player System

### Task 2.1: Player Entity
**File**: `game/Umbrella/Player.h`, `Player.cpp`
**Dependencies**: Input system, collision system

**Implementation**:
1. Create `Player` class with Rectangle bounds
2. Implement physics constants (gravity, jump velocity, move speed)
3. Add velocity vector (x, y components)
4. State tracking (onGround, facing direction, lives, score)
5. Coyote time and jump buffering

**Reference**: `Player.java:1-150` (physics and movement)

**API Design**:
```cpp
class Player {
private:
    Rect bounds;
    Pointf velocity;
    int facing;          // -1 left, 1 right
    bool onGround;
    float coyoteTimer;
    float jumpBufferTimer;
    int lives;
    int score;

    // Physics constants
    static constexpr float MOVE_SPEED = 140.0f;
    static constexpr float GRAVITY = -490.0f;
    static constexpr float MAX_FALL_SPEED = -280.0f;
    static constexpr float JUMP_VELOCITY = 280.0f;
    static constexpr float MIN_JUMP_VELOCITY = 98.0f;

public:
    Player(float x, float y, float width, float height);
    void Update(float delta, const InputState& input, CollisionHandler& collision);
    void Render(Draw& w, Point cameraOffset);

    Rect GetBounds() const { return bounds; }
    int GetLives() const { return lives; }
    int GetScore() const { return score; }
};
```

**Physics Behavior** (from Java):
- Horizontal movement: instant velocity (no lerping)
- Gravity: constant -490 pixels/sec²
- Jump velocity: 280 pixels/sec (full), 98 pixels/sec (min on release)
- Coyote time: 0.1s after leaving ground
- Jump buffer: 0.12s before landing

---

### Task 2.2: Input System
**File**: `game/Umbrella/InputController.h`, `InputController.cpp`

**Implementation**:
1. Create `InputState` struct
2. Poll keyboard state each frame
3. Track pressed vs held keys (jumpPressed vs jumpHeld)

**Reference**: `InputController.java:1-37`

**API Design**:
```cpp
struct InputState {
    bool moveLeft;
    bool moveRight;
    bool jumpHeld;
    bool jumpPressed;
    bool attackPressed;
    bool glideHeld;
    bool pausePressed;

    void Update();  // Poll Upp::GetKeyState()
};

class InputController {
private:
    InputState state;
    dword lastKeys;

public:
    const InputState& Update();
};
```

**Key Mappings**:
- Left: A / Left Arrow
- Right: D / Right Arrow
- Jump: Space / W / Up Arrow
- Attack: Right Ctrl
- Glide: Right Ctrl (hold)
- Pause: Escape

---

### Task 2.3: Player Collision Detection
**File**: `Player.cpp` (collision methods)

**Implementation**:
1. Tile-based collision using `LayerManager`
2. Multi-step collision (handle high velocities)
3. Ground detection for `onGround` state
4. Wall slide detection

**Reference**: `Player.java:180-250` (collision resolution)

**API Design**:
```cpp
class CollisionHandler {
public:
    virtual bool IsFullBlockTile(int col, int row) = 0;
    virtual bool IsWallTile(int col, int row) = 0;
    virtual bool IsFloorTile(int col, int row) = 0;
    virtual float GetGridSize() = 0;
};

// In Player class:
private:
    bool IsTouchingWallOnLeft(CollisionHandler& collision);
    bool IsTouchingWallOnRight(CollisionHandler& collision);
    void ResolveCollisionX(float deltaX, CollisionHandler& collision);
    void ResolveCollisionY(float deltaY, CollisionHandler& collision);
```

**Collision Algorithm**:
1. Move in small steps (max 3.5 pixels per step)
2. Check tile collisions at each step
3. Stop movement on collision
4. Update `onGround` by checking floor tiles below player

---

## Phase 3: Enemy Systems

### Task 3.1: Base Enemy Class
**File**: `game/Umbrella/Enemy.h`, `Enemy.cpp`

**Implementation**:
1. Create abstract `Enemy` base class
2. Rectangle bounds and velocity
3. Health, state (alive/dead/stunned)
4. Collision with player (damage/defeat)

**Reference**: `Enemy.java` (base enemy interface)

**API Design**:
```cpp
class Enemy {
protected:
    Rect bounds;
    Pointf velocity;
    int health;
    bool alive;
    float stateTimer;

public:
    virtual ~Enemy() {}
    virtual void Update(float delta, const Player& player, CollisionHandler& collision) = 0;
    virtual void Render(Draw& w, Point cameraOffset) = 0;

    bool IsAlive() const { return alive; }
    Rect GetBounds() const { return bounds; }
    void TakeDamage(int amount);
};
```

---

### Task 3.2: EnemyPatroller
**File**: `game/Umbrella/EnemyPatroller.h`, `EnemyPatroller.cpp`

**Implementation**:
1. Walks back and forth on platforms
2. Turns around at walls or edges
3. Falls off edges (no jump)

**Reference**: `EnemyPatroller.java`

**Behavior**:
- Walk speed: 70 pixels/sec
- Turns at walls (TILE_WALL collision)
- Turns at edges (no floor ahead)
- Gravity applies normally

---

### Task 3.3: EnemyJumper
**File**: `game/Umbrella/EnemyJumper.h`, `EnemyJumper.cpp`

**Implementation**:
1. Jumps periodically while walking
2. Jump timer (1.5-2.5s random)
3. Same turning logic as Patroller

**Reference**: `EnemyJumper.java`

**Behavior**:
- Walk speed: 80 pixels/sec
- Jump velocity: 300 pixels/sec
- Jump every 1.5-2.5 seconds
- Jumps when blocked by wall

---

### Task 3.4: EnemyShooter
**File**: `game/Umbrella/EnemyShooter.h`, `EnemyShooter.cpp`
**Dependencies**: Projectile class

**Implementation**:
1. Stationary enemy that shoots projectiles
2. Detects player in line of sight
3. Fires projectiles at player

**Reference**: `EnemyShooter.java`

**Behavior**:
- Stationary (no movement)
- Shoot cooldown: 2.0 seconds
- Projectile speed: 200 pixels/sec
- Range: ~15 tiles

---

### Task 3.5: Enemy Spawning
**File**: `GameScreen.cpp` (spawn methods)

**Implementation**:
1. Read spawn points from level JSON
2. Create enemy instances based on type
3. Add to enemy array

**Reference**: `GameScreen.java:200-250` (enemy initialization)

**API Design**:
```cpp
void LoadEnemies(const ValueArray& spawnsJson);
void SpawnEnemy(const String& type, float x, float y);
```

---

## Phase 4: Physics and Collision System

### Task 4.1: Collision Handler Implementation
**File**: `GameScreen.cpp` (CollisionHandler)

**Implementation**:
1. Implement `CollisionHandler` interface using `LayerManager`
2. Query terrain layer for tile types
3. Convert pixel coords to tile coords

**API Design**:
```cpp
class GameScreenCollision : public CollisionHandler {
private:
    LayerManager& layers;

public:
    bool IsFullBlockTile(int col, int row) override;
    bool IsWallTile(int col, int row) override;
    bool IsFloorTile(int col, int row) override;
    float GetGridSize() override { return 14.0f; }
};
```

---

### Task 4.2: Entity-Entity Collision
**File**: `GameScreen.cpp` (collision detection)

**Implementation**:
1. Check player bounds vs enemy bounds
2. Handle player damage on collision
3. Handle enemy defeat (parasol attack)
4. Knockback on hit

**Reference**: `GameScreen.java:400-450` (collision resolution)

**API Design**:
```cpp
void CheckPlayerEnemyCollision();
void HandlePlayerDamage(Enemy* enemy);
void HandleEnemyDefeat(Enemy* enemy);
```

---

## Phase 5: Game State and HUD

### Task 5.1: Game State Management
**File**: `GameScreen.h`, `GameScreen.cpp`

**Implementation**:
1. Add `GameState` enum (PLAYING, PAUSED, LEVEL_COMPLETE, GAME_OVER)
2. Pause screen overlay
3. Level complete screen
4. Game over screen with retry

**Reference**: `GameScreen.java:92-96` (GameState enum)

**API Design**:
```cpp
enum GameState {
    PLAYING,
    PAUSED,
    LEVEL_COMPLETE,
    GAME_OVER
};

GameState state;

void SetState(GameState newState);
void HandlePause();
void HandleLevelComplete();
void HandleGameOver();
```

---

### Task 5.2: HUD Implementation
**File**: `game/Umbrella/GameHUD.h`, `GameHUD.cpp`

**Implementation**:
1. Create `GameHUD` class (ParentCtrl)
2. Display lives (heart icons)
3. Display score
4. Display level timer (optional)
5. Overlay on top of game view

**Reference**: `HUD.java` (UI rendering)

**API Design**:
```cpp
class GameHUD : public ParentCtrl {
private:
    Label livesLabel;
    Label scoreLabel;

public:
    void SetLives(int lives);
    void SetScore(int score);
    void Paint(Draw& w) override;
};
```

---

## Phase 6: Integration and Polish

### Task 6.1: World Select Screen
**File**: `game/Umbrella/WorldSelectScreen.h`, `WorldSelectScreen.cpp`

**Implementation**:
1. Create world selection menu
2. List available worlds from level manifest
3. Launch GameScreen with selected world

**Reference**: `WorldSelectScreen.java`

---

### Task 6.2: Level Transitions
**File**: `GameScreen.cpp`

**Implementation**:
1. Detect level complete condition (reach goal)
2. Show level complete screen
3. Load next level in sequence
4. Return to world select on world complete

---

### Task 6.3: Audio Integration
**File**: `GameScreen.cpp`

**Implementation**:
1. Use existing `AudioSystem.cpp`
2. Play jump sound
3. Play enemy defeat sound
4. Play level music
5. Play UI sounds

---

## Technical Notes

### U++ Rendering Approach
- Use `ImageDraw` for offscreen rendering buffer
- Render tiles as colored rectangles (placeholder until sprites)
- Use `Draw::DrawRect()` for entities (player = blue, enemies = red/green/yellow)
- Overlay HUD using `ParentCtrl` on top of canvas

### Fixed Timestep Game Loop
```cpp
void GameScreen::LayoutLoop() {
    int64 now = GetTickCount();
    double delta = (now - lastTime) / 1000.0;
    lastTime = now;

    accumulator += delta;

    while(accumulator >= FIXED_TIMESTEP) {
        GameTick(FIXED_TIMESTEP);  // 60 FPS physics
        accumulator -= FIXED_TIMESTEP;
    }

    Refresh();  // Trigger Paint()
    SetTimeCallback(16, [=] { LayoutLoop(); });  // ~60 FPS
}
```

### Coordinate Systems
- World space: pixels (14px per tile)
- Tile space: grid coordinates (col, row)
- Screen space: viewport-relative pixels
- Conversion: `worldPos - cameraOffset = screenPos`

---

## Dependencies

### Existing Systems
- `LayerManager` - tile map storage
- `MapSerializer` - level loading
- `LevelLoader` - level file access
- `AudioSystem` - sound playback

### New Systems Required
- `Player` - player entity and physics
- `Enemy` hierarchy - enemy AI
- `InputController` - keyboard input
- `GameHUD` - UI overlay
- `CollisionHandler` - tile collision queries

---

## Estimated Complexity

| Phase | Lines of Code | Difficulty | Time Estimate |
|-------|--------------|------------|---------------|
| Phase 1 | ~300 | Medium | 4-6 hours |
| Phase 2 | ~500 | High | 8-10 hours |
| Phase 3 | ~600 | High | 10-12 hours |
| Phase 4 | ~200 | Medium | 4-6 hours |
| Phase 5 | ~400 | Medium | 6-8 hours |
| Phase 6 | ~300 | Low | 4-6 hours |
| **Total** | **~2300** | **High** | **36-48 hours** |

---

## Testing Strategy

### Unit Testing
- Test player physics in isolation (jump, gravity, collision)
- Test enemy AI state machines
- Test collision detection algorithms

### Integration Testing
- Load level and verify all entities spawn correctly
- Test player-enemy interactions
- Test level transitions

### Playtest Checklist
- [ ] Player can move, jump, and land correctly
- [ ] Player collides with walls and floors
- [ ] Enemies patrol and turn at edges
- [ ] Enemies damage player on contact
- [ ] Player can defeat enemies (parasol attack)
- [ ] Lives decrease on damage
- [ ] Game over triggers at 0 lives
- [ ] Level complete triggers at goal
- [ ] HUD displays correctly
- [ ] Audio plays appropriately

---

## Next Steps

1. **Start with Phase 1, Task 1.1**: Create GameScreen foundation
2. **Focus on rendering first**: Get tiles visible before physics
3. **Implement player controls**: Core gameplay loop
4. **Add one enemy type**: Validate enemy system
5. **Iterate and polish**: Refine physics feel

---

## References

### Java Source Files
- `RainbowGame/trash/core/src/main/java/com/rainbowgame/ui/screens/GameScreen.java`
- `RainbowGame/trash/core/src/main/java/com/rainbowgame/gameplay/components/Player.java`
- `RainbowGame/trash/core/src/main/java/com/rainbowgame/gameplay/components/Enemy*.java`
- `RainbowGame/trash/core/src/main/java/com/rainbowgame/input/InputController.java`
- `RainbowGame/trash/core/src/main/java/com/rainbowgame/camera/CameraSystem.java`

### U++ Documentation
- `uppsrc/CtrlLib` - GUI controls and TopWindow
- `uppsrc/Draw` - Drawing primitives
- `uppsrc/Core` - Timing and callbacks
