# Task D2: Droplet Spawn System

## Priority: HIGH — Tier 1 (core collectible)

## Overview
`Droplet.h/.cpp` already exists with physics and rendering. What is missing
is a spawn system: droplet sources in the level, periodic emission, and
collection by the player.

## Dependencies
- Task A4 (Level loading) — spawn points need to come from level JSON or be
  hardcoded per-level for now

## Current State
- `Droplet` class: position, velocity, bounce physics, render as colored circle
- `GameScreen` has a `Vector<Droplet*> droplets` and `UpdateDroplets()` /
  `RenderDroplets()` calls — need to verify these exist; if not, add them
- No spawn sources; no droplet-player collision

## Implementation Steps

1. **Spawn sources** — For now hardcode a few `{col, row}` emitter positions
   per level (same approach as enemy spawns). Add `DropletEmitter` struct:
   ```cpp
   struct DropletEmitter { int col, row; float timer = 0.f; };
   Vector<DropletEmitter> dropletEmitters;
   ```

2. **Periodic emission** — In `GameTick`, advance each emitter timer; when
   `timer >= DROPLET_INTERVAL` (e.g. 3.0 s), spawn a `Droplet` at that
   tile's world position with a small upward velocity.

3. **Droplet-player collision** — In `UpdateDroplets`, check each active
   droplet's bounds against player bounds; on overlap call
   `player.AddScore(15)` and deactivate the droplet.

4. **HUD counter** — Track `dropletsCollected` on `GameScreen`; render in
   HUD alongside score and lives.

5. **Level JSON support** (optional follow-up) — Add `"droplets": [{col,row}]`
   to level format; `LevelLoader` populates `dropletEmitters`.

## Files to Modify
- `game/Umbrella/GameScreen.h` — `DropletEmitter`, `dropletEmitters`,
  `dropletsCollected`
- `game/Umbrella/GameScreen.cpp` — emission tick, collision, HUD counter
- `game/Umbrella/Droplet.h/.cpp` — verify/add `IsActive()`, `GetBounds()`

## Acceptance Criteria
- Droplets periodically appear at emitter positions, bounce on floor tiles
- Player walking into a droplet collects it (score +15, droplet disappears)
- HUD shows droplet count for the current level
