# Next Implementation Tasks

**Status**: Ready for implementation
**Date**: 2026-02-15 (updated)

---

## Completed

1. ✓ **Track 6.2**: Parasol Attack System
   - Parasol state machine (IDLE, ATTACKING, GLIDING)
   - Glide/slow fall, capture-and-throw mechanics

2. ✓ **Track 6.3**: Rewards/Treats System

3. ✓ **Track 2 (AI)**: A* Pathfinder + NavGraph + ActionSet + AIController + EnemyBehaviors
   - All three enemy types wired to AI behaviors
   - EnemyPatroller → WandererBehavior
   - EnemyJumper → StalkerBehavior
   - EnemyShooter → ShooterBehavior
   - Projectile speed reduced to 90 px/s

---

## Remaining High-Priority Tasks (from session)

### Task 2: Droplet System Overhaul + Water Weapon (Track 6.1)
**Priority**: HIGH (Core collectible + combat mechanic)
**Estimated**: 16-20 hours

**Scope**: Physics overhaul (80s arcade style) + merge-at-5 huge droplet +
throw on release + water weapon (grid-stepping snake) + enemy attachment.

**See**: `plan/implementation/development/task30/Task_D2_DropletSpawnSystem.md`
for full implementation plan.

**Files to create**:
- `game/Umbrella/WaterWeapon.h`
- `game/Umbrella/WaterWeapon.cpp`

**Files to modify**:
- `game/Umbrella/Droplet.h` — arcade physics constants, `isHuge`/`thrown` flags
- `game/Umbrella/Droplet.cpp` — constant-speed fall, no bounce
- `game/Umbrella/GameScreen.h` — WaterWeapon member, `hasHugeDroplet`
- `game/Umbrella/GameScreen.cpp` — merge-at-5, throw/water dispatch, water update

---

### Task 3: Level Goal System (Track 10.2)
**Priority**: HIGH (Win condition)
**Estimated**: 8-10 hours

**Implementation Plan**:
1. Add goal tile type to `TileType` enum (TILE_GOAL or TILE_EXIT)
2. Parse goal position from level data
3. Detect player reaching goal (collision with goal tile)
4. Calculate level completion score:
   - Time bonus (faster = more points)
   - No-damage bonus
   - All-droplets bonus
5. Trigger LEVEL_COMPLETE state
6. Show completion screen with stats
7. Progress to next level (if available)

**Files to modify**:
- `game/Umbrella/Tile.h` (add TILE_GOAL)
- `game/Umbrella/GameScreen.cpp` (goal detection, completion)

**Integration**:
- Goal renders as bright green/gold tile
- Level complete screen shows:
  - Time taken
  - Score earned
  - Droplets collected
  - Damage taken
  - Grade/stars (S/A/B/C)

---

### Task 4: Pickup System (Track 6.4)
**Priority**: MEDIUM (Health/powerups)
**Estimated**: 6-8 hours

**Implementation Plan**:
1. Create `Pickup` class with type enum (HEART, GEM, LIGHTNING, SPEED)
2. Spawn pickups in level (hardcode positions for now)
3. Implement pickup-player collision
4. Apply effects:
   - HEART: Restore 1 life
   - GEM: +100 score
   - LIGHTNING: Temporary invincibility (5 seconds)
   - SPEED: Movement speed boost (10 seconds)
5. Add temporary effect tracking to Player
6. Render pickups as colored icons

**Files to create**:
- `game/Umbrella/Pickup.h`
- `game/Umbrella/Pickup.cpp`

**Integration**:
- Pickups float/pulse for visibility
- Effect icons show in HUD when active
- Speed boost multiplier: 1.5x movement speed

---

### Task 5: Audio Integration (Track 12.3)
**Priority**: MEDIUM (Feedback)
**Estimated**: 6-8 hours

**Implementation Plan**:
1. Create sound effect catalog (map IDs to file paths)
2. Load sound assets in AudioSystem
3. Add playback calls in gameplay:
   - Player jump: "jump.wav"
   - Player land: "land.wav"
   - Player hit: "hit.wav"
   - Enemy defeat: "enemy_defeat.wav"
   - Treat collect: "coin.wav"
   - Droplet collect: "droplet.wav"
   - Parasol attack: "whoosh.wav"
   - Level complete: "victory.wav"
4. Add background music playback
5. Add volume controls (in settings, not immediate)

**Files to modify**:
- `game/Umbrella/AudioSystem.cpp` (add playback methods)
- `game/Umbrella/Player.cpp` (play jump/land/hit sounds)
- `game/Umbrella/GameScreen.cpp` (play defeat/collect sounds)

**Integration**:
- Use existing AudioSystem.cpp infrastructure
- Sound files can be placeholders (silence/simple tones) for testing
- Real sound effects can be added later

---

## Implementation Order Recommendation

### Tier 1 — Core game loop (makes it actually playable)

1. **Level complete + next-level progression** (Track 10.2)
   - Detect all enemies dead → trigger LEVEL_COMPLETE (partially done)
   - Load and transition to next level file
   - "Level clear" overlay with score summary
   - Files: `GameScreen.cpp`, `LevelLoader.cpp`

2. **Water/Droplet System** (Track 6.1, 12-16 hrs) — Core collectible mechanic
   - `Droplet` class exists but no spawn system
   - Parse droplet spawn points, bounce physics, player collection
   - HUD droplet counter

### Tier 2 — Polish / feel

3. **Lives + proper game over** (Track 6.x)
   - Player has `lives` counter; `GAME_OVER` state exists
   - Need: decrement lives on death, show lives in HUD, game-over screen on 0 lives
   - Files: `Player.cpp`, `GameScreen.cpp`

4. **Pickup System** (Track 6.4, 6-8 hrs) — Health/powerups
   - HEART (restore life), GEM (score), LIGHTNING (temp invincibility)
   - Files: new `Pickup.h/.cpp`, `GameScreen.cpp`

5. **Audio Integration** (Track 12.3, 6-8 hrs) — Feedback
   - Hook AudioSystem into gameplay events (jump, defeat, collect)

### Tier 3 — Content / variety

6. **FlyingEnemy** (Track 7.1) — ignores gravity, flies toward player
   - Straightforward with existing AIController (StalkerBehavior, `onGround=true` always)
   - New `EnemyFlyer.h/.cpp`

7. **GrimReaper boss** (Track 7.1) — time-pressure mechanic
   - Spawns after N seconds, indestructible, chases player relentlessly
   - Forces level completion or death
   - New `EnemyReaper.h/.cpp`

---

See `MissingFeatures.md` for full roadmap.
