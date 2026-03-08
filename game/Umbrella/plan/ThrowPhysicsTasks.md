# Enemy Throwing Physics - Implementation Tasks

**Status**: Ready for implementation
**Priority**: HIGH (Core mechanic is broken)
**Date**: 2026-02-08

---

## Problem

Currently, thrown enemies are released but don't move horizontally. They just fall straight down.

---

## Requirements (from USER_EXPERIENCE.md)

### Deactivated Enemy Throwing:
- Player throws deactivated enemy **horizontally** left or right (based on facing direction)
- Thrown enemy travels horizontally until hitting a **wall**
- On wall impact, enemy is **destroyed**
- Thrown enemy **takes with it** living enemies of same size or smaller
- Also takes with it all deactivated enemies regardless of size
- All carried enemies are destroyed when hitting wall

### Visual Effects:
1. **Deactivated Enemy** (when captured on umbrella):
   - Greenish tint (grayscale + green)
   - Spins/rotates animation
   - Flying with "intensity"

2. **Dead Enemy** (when killed):
   - Red tint (grayscale + red)
   - Flies in some direction
   - Lands on ground
   - Turns into edible treat on ground impact

---

## Implementation Tasks

### Task 1: Fix Thrown Enemy Horizontal Movement (URGENT)
**Priority**: CRITICAL
**Estimated**: 2 hours

**Problem**: Enemy.cpp ThrowFrom() currently applies both horizontal AND upward velocity, causing arc motion instead of pure horizontal.

**Fix**:
1. Remove upward velocity component (`throwVY`)
2. **Thrown (deactivated) enemies** should move **purely horizontally** with gravity (not arc)
3. **Dead enemies** (after wall hit) SHOULD move in arc - that's correct behavior
4. Current code in Enemy::ThrowFrom():
   ```cpp
   velocity.x = vx;
   velocity.y = vy;  // <-- Set to 0 for thrown enemies, but keep arc for dead enemies
   ```

**Important Distinction**:
- **Thrown state**: Horizontal flight until wall hit
- **Dead state**: Arc motion after wall hit, lands on ground

**Files to modify**:
- `game/Umbrella/Enemy.cpp` - ThrowFrom() method (set vy=0 for thrown)
- `game/Umbrella/GameScreen.cpp` - Remove THROW_VELOCITY_Y from throw call

---

### Task 2: Thrown Enemy Collision Detection
**Priority**: HIGH
**Estimated**: 3-4 hours

**Implementation**:
1. Add `thrown` state tracking to Enemy (already exists)
2. When `thrown=true`, check for wall collisions:
   - Use `IsWallTile()` or `IsFullBlockTile()` from collision handler
   - Check in direction of movement (left or right)
3. On wall hit:
   - Set enemy to dead
   - Spawn treat at collision point
   - Award bonus score (200 points as currently coded)

**Files to modify**:
- `game/Umbrella/Enemy.cpp` - Add wall collision check in Update()
- `game/Umbrella/GameScreen.cpp` - Already has thrown enemy logic

---

### Task 3: Thrown Enemy Takes Others With It
**Priority**: MEDIUM
**Estimated**: 4-5 hours

**Implementation**:
1. During thrown enemy movement, check collision with other enemies
2. If collision detected:
   - Living enemy: Check size comparison
     - If thrown enemy size >= target size: capture target
   - Deactivated enemy: Always capture regardless of size
3. Captured enemies:
   - Move with thrown enemy (same velocity)
   - Set their `captured=true` flag
   - Disable their normal AI
4. On wall impact:
   - Kill all captured enemies
   - Spawn treats for each
   - Award score for each

**New fields needed**:
- `Enemy::carriedEnemies` - Array of enemies being carried
- Or simpler: Just set velocity of hit enemies to match thrower

**Files to modify**:
- `game/Umbrella/Enemy.cpp` - Add collision check in Update()
- `game/Umbrella/GameScreen.cpp` - Handle group destruction

---

### Task 4: Deactivated Enemy Visual Effects
**Priority**: MEDIUM (Polish)
**Estimated**: 3-4 hours

**Implementation**:
1. Add `rotation` field to Enemy
2. When `captured=true` or `thrown=true`:
   - Spin rotation angle each frame
   - `rotation += 360.0f * delta * 2.0f;` // 2 rotations per second
3. Add color tint:
   - Deactivated (greenish): `Color(0, 255, 100)` multiplied with base color
   - Dead (reddish): `Color(255, 50, 50)` multiplied with base color
4. Render with rotation and tint

**Files to modify**:
- `game/Umbrella/Enemy.h` - Add rotation field
- `game/Umbrella/Enemy.cpp` - Update rotation in Update()
- `game/Umbrella/Enemy.cpp` - Apply rotation/tint in Render()

---

### Task 5: Dead Enemy Physics and Treat Spawning
**Priority**: MEDIUM
**Estimated**: 2-3 hours

**Implementation**:
1. When enemy dies, set `dead=true` state
2. Apply physics:
   - Random velocity in some direction
   - Gravity applies
   - Red tint applied
3. On ground collision:
   - Spawn treat at impact point
   - Remove enemy from world

**Files to modify**:
- `game/Umbrella/Enemy.cpp` - Add death state and physics
- `game/Umbrella/GameScreen.cpp` - Handle treat spawning on ground impact

---

## Implementation Order

1. **Task 1** - Fix horizontal throwing (CRITICAL - 2 hours)
2. **Task 2** - Wall collision detection (HIGH - 3-4 hours)
3. **Task 3** - Capture other enemies (MEDIUM - 4-5 hours)
4. **Task 4** - Visual effects (MEDIUM - 3-4 hours)
5. **Task 5** - Dead enemy physics (MEDIUM - 2-3 hours)

**Total Estimated**: 14-18 hours

---

## Notes

- Task 1 is CRITICAL and should be done immediately
- Tasks 2-3 complete core throwing mechanic
- Tasks 4-5 are polish but add significant game feel

---

## Related Documentation

- `/common/active/sblo/Dev/RainbowGame/trash/USER_EXPERIENCE.md` - Full throwing mechanics description
- `game/Umbrella/Enemy.h` - Current enemy state
- `game/Umbrella/Player.h` - THROW_VELOCITY constants
