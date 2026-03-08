# Task D2: Droplet System Overhaul + Water Weapon

## Priority: HIGH — Tier 1 (core collectible + combat mechanic)

## Overview

The droplet system needs a complete physics overhaul from modern gravity to 80s
arcade style, plus a new water weapon mechanic triggered by collecting 5 droplets.

The spawning and collection infrastructure already exists (emitters, orbit,
HUD counter). What needs to change is:

1. **Physics**: constant-speed falling instead of accelerating gravity
2. **Merge at 5**: 5 orbiting droplets merge into 1 huge droplet (3× size)
3. **Throw on release**: releasing action button with <5 droplets throws them horizontally
4. **Water weapon**: releasing action button with huge droplet (5+) triggers
   a grid-stepping water snake that destroys enemies

## Dependencies

- Droplet spawning already works (DropletSpawnPoint, emitter timers in GameTick)
- Droplet-player collision and orbit already work
- Treat system exists for enemy death rewards

## Current State

- `Droplet` class exists with accelerating gravity (-490 px/s²), bounce, friction
- `GameScreen` has full spawn/collect/orbit pipeline (lines 594-679)
- Droplet size 6px radius = 12px diameter (matches player's 12px width)
- Missing: constant-speed physics, merge-at-5, throw, water weapon

---

## Part 1: Droplet Physics Overhaul

### Files: `Droplet.h`, `Droplet.cpp`

**Replace constants**:
```cpp
// OLD — remove these:
static constexpr float GRAVITY = -490.0f;
static constexpr float MAX_FALL_SPEED = -400.0f;
static constexpr float BOUNCE_DAMPING = 0.6f;
static constexpr float MIN_BOUNCE_VY = 50.0f;

// NEW — arcade-style:
static constexpr float FALL_SPEED = 60.0f;        // Constant downward speed (px/s)
static constexpr float HORIZONTAL_SPEED = 50.0f;   // Horizontal throw speed (px/s)
```

**Rewrite `Droplet::Update()`**:
- Set `velocity.y = -FALL_SPEED` (constant, not accelerating)
- On ground collision: `velocity.y = 0`, snap to ground (no bounce)
- On wall collision: reverse `velocity.x` direction (keep same magnitude)
- Remove friction (`velocity.x *= 0.95f`)

**Add `isHuge` flag** to Droplet:
- `bool isHuge = false;` — set when 5 droplets merge
- When huge: `size = DROPLET_SIZE * 3` (18px radius = 36px diameter)
- `void MakeHuge()` — sets flag, triples size
- Render huge droplet with brighter/pulsing effect

---

## Part 2: Merge at 5 Droplets

### Files: `GameScreen.cpp`

In the droplet collection section (around line 657-679):

When `dropletsCollected` reaches 5:
1. Remove all 5 orbiting (collected) droplets from the array
2. Spawn 1 new droplet with `MakeHuge()` in collected/orbit state
3. Reset `dropletsCollected` to 0 (the 5 are consumed)
4. Track `hasHugeDroplet = true` on GameScreen

**Visual**: Huge droplet orbits player like normal but is 3× larger.

---

## Part 3: Throw on Action Release (< 5 droplets)

### Files: `GameScreen.cpp`

When action button is released and player has orbiting droplets (but no huge droplet):

1. For each collected droplet:
   - Set `collected = false`
   - Set `velocity.x = player.GetFacing() * HORIZONTAL_SPEED`
   - Set `velocity.y = 0` (no gravity when thrown horizontally)
2. Droplets fly horizontally, stop at walls or go off-screen

**Add a thrown state to Droplet** (`bool thrown = false`):
- When thrown: skip gravity entirely, only move horizontally
- On wall hit: deactivate
- On off-screen: deactivate

---

## Part 4: Water Weapon (Snake) — New System

### New files: `WaterWeapon.h`, `WaterWeapon.cpp`

**Activation**: Player releases action button while holding huge droplet.

**Grid-step movement** (like old-school Snake):
- Position tracked in grid coordinates (col, row)
- Moves one grid square at a time at ~5 steps/second
- Step timer accumulates delta until >= stepInterval (0.2s)

**Movement rules**:
```
Each step:
  1. If in falling mode:
     - Check tile at (col, row-1) — one below
     - If empty → move down: row--
     - If solid → switch to horizontal mode

  2. If in horizontal mode:
     - Check tile at (col + directionX, row) — next horizontal
     - If empty → move there, then check if tile below is empty:
       - If below is empty → switch back to falling mode
     - If blocked → reverse directionX (mirror)

  3. If row < 0 (below map) → destroy water weapon, release enemies
```

**Data structure**:
```cpp
class WaterWeapon : public GameEntity {
    int col, row;              // Grid position
    int directionX;            // -1 or +1
    bool falling;              // true = moving down, false = horizontal
    float stepTimer;           // Accumulator
    bool active;

    static constexpr float STEP_INTERVAL = 0.2f;  // ~5 steps/sec
    static constexpr int TRAIL_LENGTH = 8;

    Vector<Point> trail;       // Last N grid positions (visual)
    Vector<Enemy*> attached;   // Enemies riding the water

    void Step(Player::CollisionHandler& collision);
    void AttachEnemy(Enemy* e);
    void Release();  // Destroy weapon, launch enemies upward
};
```

**Rendering**: Draw filled colored squares at current position + trail
(trail fades from bright cyan to transparent over TRAIL_LENGTH steps).

**Enemy attachment**:
- Each step, check if any alive enemy overlaps current grid square
- If so: enemy becomes attached, stops its own AI, position locked to water
- Attached enemies move with the water each step

---

## Part 5: Enemy Death-by-Water Arc

When water weapon is destroyed (falls below map):
- Each attached enemy:
  1. Position teleported to top of level (row = levelRows + 2)
  2. Set velocity: `vx = random(-80, 80)`, `vy = 200` (upward arc)
  3. Enable normal gravity on the dead enemy body
  4. When dead enemy lands on ground → spawn treat (reuses existing
     dead-enemy treat logic in GameScreen lines 311-352)

This reuses the existing dead-enemy-spawns-treat-on-ground-contact code
already in GameTick.

---

## Files Summary

| File | Action |
|------|--------|
| `game/Umbrella/Droplet.h` | Replace gravity constants, add `isHuge`, `thrown` flags |
| `game/Umbrella/Droplet.cpp` | Rewrite Update() for arcade physics, add MakeHuge() |
| `game/Umbrella/WaterWeapon.h` | **New** — WaterWeapon class |
| `game/Umbrella/WaterWeapon.cpp` | **New** — grid movement, enemy attachment, trail |
| `game/Umbrella/GameScreen.h` | Add WaterWeapon member, `hasHugeDroplet` flag |
| `game/Umbrella/GameScreen.cpp` | Merge-at-5, throw/water dispatch on release, water update/render |
| `game/Umbrella/Umbrella.upp` | Add WaterWeapon.h/.cpp |

## Implementation Order

1. Droplet physics overhaul (constant speed, no bounce)
2. Thrown state (horizontal-only movement)
3. Merge at 5 → huge droplet
4. Action release dispatch (throw vs water weapon)
5. WaterWeapon class (grid movement + rendering)
6. Enemy attachment to water
7. Death arc animation (enemies fly up, land as treats)

## Acceptance Criteria

- [ ] Droplets fall at constant speed, no acceleration or bounce
- [ ] Collecting 5 droplets merges them into 1 huge (3×) droplet
- [ ] Releasing action with <5 orbiting droplets throws them horizontally
- [ ] Releasing action with huge droplet triggers water weapon
- [ ] Water weapon moves grid-square-by-square downward
- [ ] Water weapon bounces horizontally off walls (reverses direction)
- [ ] Water weapon falls through empty spaces
- [ ] Enemies touching water get attached and ride it
- [ ] Water destroyed at bottom → enemies arc up → land as treats
- [ ] HUD shows droplet count correctly through all phases

## Verification

1. Build: `script/build.py -mc 1 -j 12 Umbrella`
2. Play: droplets should fall at constant speed from emitters
3. Collect 4 droplets → release action → thrown horizontally
4. Collect 5th → auto-merge into huge droplet orbiting player
5. Release action with huge → water snake starts at player position
6. Snake falls, hits floor, moves sideways, bounces off walls
7. Snake picks up enemies on contact
8. Snake falls through hole at bottom → enemies arc up → treats spawn
