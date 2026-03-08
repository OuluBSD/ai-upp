# Task D6: EnemyFlyer — Flying Enemy

## Priority: MEDIUM — Tier 3 (content variety)

## Overview
A flying enemy that ignores gravity and pursues the player through the air.
Uses the existing `AIController` + `StalkerBehavior` with gravity disabled.

## Java Reference
`gameplay/enemies/FlyingEnemy.java`, `gameplay/enemies/AirbornePathEnemy.java`

## Design
- Floats at fixed altitude offset above its spawn row
- Moves horizontally toward player; can cross gaps and fly over walls
- No jumping — moves directly in 8 directions (or just horizontal + vertical)
- Defeated by parasol hit (same as ground enemies)
- Drop treat on death; no throw mechanic (too light)

## Implementation Steps

1. **`EnemyFlyer` class** — `EnemyFlyer.h/.cpp`:
   - Inherits `Enemy`
   - `ENEMY_FLYER` type ID
   - No gravity in `Update()` — set `velocity.y` each frame based on target
   - `WireAI()` sets `StalkerBehavior` (already tracks player position)

2. **Movement** — In `Update()`:
   - Get player center and self center
   - Set `velocity.x = (playerX > selfX) ? FLY_SPEED : -FLY_SPEED`
   - Set `velocity.y = (playerY > selfY) ? FLY_SPEED : -FLY_SPEED`
   - No `ResolveCollisionY` (flies through gaps); keep `ResolveCollisionX` to
     avoid embedding in walls, or remove it entirely for phase-through walls
   - Alternatively: use AIController `ACT_LEFT`/`ACT_RIGHT` for horizontal;
     add new `ACT_UP`/`ACT_DOWN` actions — simpler to just do direct tracking

3. **Render** — Purple rectangle base color; facing indicator same as others

4. **Spawn wiring** — Add `ENEMY_FLYER` to `GameScreen::SpawnEnemies()` type
   dispatch and to `LevelLoader` type parsing

5. **Parasol defeat** — Reuses existing parasol-hitbox collision in
   `GameScreen`; no changes needed if type is handled in the same loop

## Files to Create
- `game/Umbrella/EnemyFlyer.h`
- `game/Umbrella/EnemyFlyer.cpp`

## Files to Modify
- `game/Umbrella/GameScreen.cpp` — add `ENEMY_FLYER` spawn case
- `game/Umbrella/Umbrella.upp` — add EnemyFlyer.h / EnemyFlyer.cpp

## Acceptance Criteria
- Flyer spawns in level, floats toward player position
- Cannot be blocked by floor tiles (flies over them)
- Defeated by parasol hit; drops treat
