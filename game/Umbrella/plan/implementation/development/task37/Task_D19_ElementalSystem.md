# Task D19: Elemental System — Water/Fire/Lightning/Star Super Powers

## Priority: HIGH — Tier 1 (core combat mechanic, boss weakness)

## Overview

Blue balls (droplets) carry an elemental type that varies per level. Collecting
5 activates the element's super power. Currently we have Water (grid-stepping
snake). This task adds Fire, Lightning, and Star elements.

## Java Reference
- `gameplay/water/DropletMode.java` (RAINBOW, ICE, FIRE)
- `~/Dev/RainbowGame/trash/USER_EXPERIENCE.md` section 3.1.2 "Events"

## Source Material: Elemental Super Powers

From USER_EXPERIENCE.md:

1. **Water**: A wave flows along the ground seeking the sewer/drain. Bounces
   off walls. Enemies hit are deactivated and travel with water to bottom.
   They fall from sky deactivated (not dead) and player must finish them.
   — **ALREADY IMPLEMENTED** (WaterWeapon.cpp)

2. **Fire**: Player throws 6 small flames horizontally in front. They are
   evenly spaced and fall until hitting a surface. They remain on the surface.
   Enemies hitting flames are deactivated and fly in an arc.

3. **Lightning**: Player throws a horizontal bolt that passes through walls
   and instantly kills all enemies in its path.

4. **Star**: Player shoots stars evenly in all directions. Stars pass through
   walls. First hit deactivates enemy, second hit kills it.

## Design

### Element Enum
```cpp
enum ElementType {
    ELEMENT_WATER,      // Existing WaterWeapon
    ELEMENT_FIRE,       // 6 flame projectiles
    ELEMENT_LIGHTNING,  // Horizontal kill beam
    ELEMENT_STAR        // Radial star burst
};
```

### Per-Level Element
Each level specifies its element in the JSON:
```json
{ "element": "water" }
```
This determines what type the droplets are and what super power activates
when 5 are merged.

### Fire Weapon
- 6 flame entities spawned in front of player, evenly spaced horizontally
- Each falls under gravity until hitting a surface
- Flames persist on surfaces for ~5 seconds
- Enemy touching flame: deactivated, flung in arc
- Visual: animated orange/red flame sprites

### Lightning Weapon
- Single horizontal beam from player position in facing direction
- Passes through all walls (infinite range to level edge)
- Instantly kills all enemies in its path
- Visual: bright yellow horizontal line with jagged edges, flash effect
- Duration: ~0.5 seconds

### Star Weapon
- 8-12 star projectiles shot evenly in all directions (radial)
- Stars pass through walls
- First hit: deactivate enemy
- Second hit: kill enemy (if already deactivated)
- Visual: spinning 5-pointed stars, yellow/white
- Stars travel until off-screen

## Implementation Steps

1. **Add `ElementType` enum** to a shared header (e.g., `GameTypes.h`)

2. **Per-level element** — Parse from level JSON in `LoadLevel()`
   - Store `ElementType levelElement` in GameScreen
   - Droplets visually tinted by element color

3. **Fire weapon** — `FireWeapon.h/.cpp`:
   - Spawns 6 `FireFlame` entities in front of player
   - Each has gravity, sticks to first surface hit
   - Damages enemies on contact (deactivate)
   - Timer-based despawn (5 seconds)

4. **Lightning weapon** — `LightningWeapon.h/.cpp`:
   - Raycast from player position in facing direction
   - Kill all enemies along the ray
   - Flash visual effect (0.5s duration)

5. **Star weapon** — `StarWeapon.h/.cpp`:
   - Spawn 8-12 star projectiles in radial pattern
   - Each moves at constant speed through walls
   - Deactivate on first hit, kill on second (track per-enemy)

6. **Dispatch in GameScreen** — When huge droplet released:
   - Check `levelElement`
   - Activate corresponding weapon

## Files to Create
- `game/Umbrella/GameTypes.h` (shared enums)
- `game/Umbrella/FireWeapon.h` / `FireWeapon.cpp`
- `game/Umbrella/LightningWeapon.h` / `LightningWeapon.cpp`
- `game/Umbrella/StarWeapon.h` / `StarWeapon.cpp`

## Files to Modify
- `game/Umbrella/GameScreen.h` — `levelElement`, weapon instances
- `game/Umbrella/GameScreen.cpp` — element dispatch, weapon update/render
- `game/Umbrella/Droplet.h/cpp` — element-based tint
- `game/Umbrella/Umbrella.upp` — add new files

## Dependencies
- D2 (Water weapon) — DONE (serves as pattern for other weapons)

## Acceptance Criteria
- [ ] Levels can specify element type in JSON
- [ ] Collecting 5 droplets activates element-specific super power
- [ ] Fire: 6 flames fall and stick to surfaces, deactivate enemies
- [ ] Lightning: horizontal beam kills all enemies in path through walls
- [ ] Star: radial burst deactivates/kills enemies, passes through walls
- [ ] Water weapon still works as before
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
