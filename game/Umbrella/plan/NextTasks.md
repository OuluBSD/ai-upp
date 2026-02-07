# Next Implementation Tasks

**Status**: Ready for implementation
**Date**: 2026-02-07

---

## Completed Today

1. ✓ **Track 6.2**: Parasol Attack System (6 hours)
2. ✓ **Track 6.3**: Rewards/Treats System (3 hours)

---

## Remaining High-Priority Tasks (from session)

### Task 2: Water/Droplet System (Track 6.1)
**Priority**: HIGH (Primary collectible mechanic)
**Estimated**: 12-16 hours

**Implementation Plan**:
1. Create `Droplet` class (position, velocity, mode enum: RAINBOW/ICE/FIRE)
2. Physics: bounce behavior, gravity, rotation
3. Create `DropletManager` to spawn and track droplets
4. Parse droplet spawn points from level (hardcode for now, annotations later)
5. Implement droplet-player collision (collect)
6. Track collected droplets in HUD
7. Render droplets as colored circles (RAINBOW=all colors, ICE=cyan, FIRE=orange)

**Files to create**:
- `game/Umbrella/Droplet.h`
- `game/Umbrella/Droplet.cpp`
- `game/Umbrella/DropletManager.h`
- `game/Umbrella/DropletManager.cpp`

**Integration**:
- Add droplet counter to HUD
- Spawn droplets at regular intervals or from specific tiles
- Collection gives score bonus (10-25 points per droplet)

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

1. **Level Goal System** (8-10 hours) - Gives immediate sense of progress
2. **Water/Droplet System** (12-16 hours) - Core collectible mechanic
3. **Pickup System** (6-8 hours) - Adds variety and player agency
4. **Audio Integration** (6-8 hours) - Polish and feedback

**Total**: 32-42 hours to complete all 4 tasks

---

## Beyond These 5 Tasks

After completing these, the game will have:
- Complete gameplay loop (move, attack, collect, reach goal)
- Score system with treats and droplets
- Health pickups and powerups
- Audio feedback
- Level completion and progression

Next major features would be:
- **Sprite/Animation System** (replace colored rectangles)
- **Additional Enemy Types** (flying, pathing, GrimReaper)
- **Annotation System** (data-driven enemy/droplet spawning)
- **Level Manifest** (world map, level select)
- **Entity Editor** (sprite authoring tools)

See `MissingFeatures.md` for full roadmap.
