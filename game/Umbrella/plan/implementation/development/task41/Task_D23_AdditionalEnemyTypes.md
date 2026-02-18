# Task D23: Additional Enemy Types — World-Specific Enemies

## Priority: MEDIUM — Tier 2 (content variety)

## Overview

RainbowGame has 30+ enemy types across 8 worlds. Currently Umbrella has 4
(Patroller, Jumper, Shooter, Flyer) + GrimReaper. This task adds the most
important missing variants needed for world variety.

## Java Reference
- `gameplay/enemies/` (all enemy classes)
- `~/Dev/RainbowGame/trash/USER_EXPERIENCE.md` (enemy descriptions per world)

## Source Material: Enemy Types Per World

### World 1 (Music Star)
- **Casta-kun**: Castanet, moves left/right, falls. Opens/closes at 2Hz.
- **Pet-kun**: Trumpet, moves sideways
- **Accorden**: Accordion, moves sideways
- **Pianon**: Piano, spawns Casta-kun (spawner enemy)
- **Sanka-kun**: Triangle, small

### World 2 (Woods Star)
- **Apple-Head**: Apple, basic walker
- **Yuni**: Unicorn, pink pony
- **Char**: Lion
- **Kirikabun**: Tree stump, spawns Apple-Head (spawner)
- **Vio-kun**: Carnivorous plant, shoots ball lightning
- **Bat-kun**: Bat, flying
- **Iga Iga**: Spiky, static hazard

### World 3-8: More variants...

## Priority Enemy Types to Add

1. **SpawnerEnemy** — Stationary, periodically spawns smaller enemies
   (covers Pianon, Kirikabun, Slotton pattern)
2. **IdleEnemy** — Stationary hazard, no AI movement
   (covers Iga Iga, static obstacles)
3. **SidewaysEnemy** — Moves only horizontally on walls/platforms
   (covers many basic enemy variants)
4. **ChargerEnemy** — Charges toward player when in line of sight
   (covers Char lion behavior)
5. **ProjectileEnemy** — Enhanced shooter with configurable projectile type
   (covers Vio-kun ball lightning, Gun-chan)

## Implementation Steps

1. **SpawnerEnemy** — `SpawnerEnemy.h/.cpp`:
   - Stationary position
   - Timer-based spawn of configurable child enemy type
   - Max children cap (e.g., 3 alive at once)
   - Animation: opens/activates when spawning

2. **IdleEnemy** — Simple: no AI, just a hitbox + damage on contact
   - Can be killed by parasol/elements
   - Useful for hazard tiles that aren't tiles

3. **SidewaysEnemy** — Like Patroller but horizontal-only variant
   - Configurable speed
   - Reverse direction at walls/edges

4. **ChargerEnemy** — `ChargerEnemy.h/.cpp`:
   - Idle until player enters detection range
   - Charges at increased speed toward player
   - Stops after covering charge distance or hitting wall
   - Cool-down before next charge

5. **Enemy type registry** — Map string names to factory functions:
   ```cpp
   Enemy* CreateEnemy(const String& type, int col, int row, int facing);
   ```
   Level JSON: `{ "type": "SPAWNER", "spawns": "PATROLLER", ... }`

## Files to Create
- `game/Umbrella/SpawnerEnemy.h` / `SpawnerEnemy.cpp`
- `game/Umbrella/IdleEnemy.h` / `IdleEnemy.cpp`
- `game/Umbrella/ChargerEnemy.h` / `ChargerEnemy.cpp`

## Files to Modify
- `game/Umbrella/Enemy.h` — add to EnemyType enum
- `game/Umbrella/GameScreen.cpp` — enemy factory, spawn from level data
- `game/Umbrella/EntityPlacementTool.cpp` — add new types to editor
- `game/Umbrella/MapSerializer.cpp` — serialize new enemy types
- `game/Umbrella/Umbrella.upp` — add new files

## Dependencies
- D20 (Sprite system) — nice to have but not blocking

## Acceptance Criteria
- [ ] SpawnerEnemy periodically creates child enemies
- [ ] IdleEnemy exists as static hazard
- [ ] ChargerEnemy detects and charges player
- [ ] New enemy types selectable in map editor
- [ ] New types serialize/deserialize in level JSON
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
