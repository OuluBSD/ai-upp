# Task D7: GrimReaper Boss — Time-Pressure Mechanic

## Priority: MEDIUM — Tier 3 (content variety)

## Overview
After a configurable time limit, the GrimReaper spawns and pursues the player
relentlessly. It is indestructible — the only way to survive is to complete
the level before it catches you. This creates urgency in long levels.

## Java Reference
`gameplay/enemies/GrimReaper.java`

## Design
- Spawns off-screen (left or right) after `REAPER_SPAWN_TIME` seconds (e.g. 90 s)
- Moves at constant horizontal speed toward player; ignores gravity
- Indestructible — parasol hit has no effect
- Touching the player instantly kills them (GAME_OVER if no lives)
- A visual/audio warning plays 10 s before spawn ("IT'S HERE")
- Despawns when level is completed

## Implementation Steps

1. **`EnemyReaper` class** — `EnemyReaper.h/.cpp`:
   - Inherits `Enemy` but overrides `Defeat()` as no-op (indestructible)
   - No parasol interaction — exempt from parasol-hitbox collision loop
   - Uses direct player-tracking movement (no AIController needed — it never
     replans, just moves toward player at fixed speed)

2. **Spawn timer in GameScreen**:
   ```cpp
   float reaperTimer = 0.f;
   EnemyReaper* reaper = nullptr;
   static constexpr float REAPER_SPAWN_TIME = 90.f;
   static constexpr float REAPER_WARNING_TIME = 10.f;
   ```
   In `GameTick()`:
   - Increment `reaperTimer` while in PLAYING state
   - At `REAPER_SPAWN_TIME - REAPER_WARNING_TIME`: show warning overlay
   - At `REAPER_SPAWN_TIME`: spawn `EnemyReaper` off left edge of screen

3. **Player collision** — In `GameTick()`, if `reaper` exists and its bounds
   overlap the player, call `HandlePlayerDeath()`.

4. **Warning overlay** — Draw a flashing red-tinted banner ("IT'S HERE") for
   the 10 s warning window.

5. **Cleanup** — `delete reaper; reaper = nullptr` on level complete or death.

## Files to Create
- `game/Umbrella/EnemyReaper.h`
- `game/Umbrella/EnemyReaper.cpp`

## Files to Modify
- `game/Umbrella/GameScreen.h` — `reaperTimer`, `reaper` pointer
- `game/Umbrella/GameScreen.cpp` — spawn logic, collision, warning overlay,
  cleanup
- `game/Umbrella/Umbrella.upp` — add EnemyReaper.h / EnemyReaper.cpp

## Acceptance Criteria
- Reaper does not spawn for the first 90 s of play
- Warning banner appears 10 s before spawn
- Reaper tracks player position, cannot be defeated
- Player dying to the Reaper uses normal death/lives flow
- Completing the level before the Reaper catches you avoids it entirely
