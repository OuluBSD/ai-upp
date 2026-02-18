# Task D18: Boss System — End-of-World Final Bosses

## Priority: HIGH — Tier 1 (core gameplay milestone)

## Overview

Each world ends with a boss level (level X.7). Bosses are XXL enemies with
unique attack patterns, high strength, and elemental weaknesses. The boss level
has special rules: dark background at start, elemental flask spawn on player
respawn, bright background + super treat on defeat.

## Java Reference
- `ui/screens/GameScreen.java` (boss detection, boss state)
- `gameplay/enemies/` (individual boss classes)
- `~/Dev/RainbowGame/trash/USER_EXPERIENCE.md` (detailed boss descriptions)

## Source Material: Boss Roster

| World | Boss | Weakness | Attack Pattern |
|-------|------|----------|----------------|
| 1. Music Star | Hyper Music'n (One Man Band) | Lightning | Musical projectiles |
| 2. Woods Star | Hyper Vio-kun (Mother Bird) | Fire | Spawns plant enemies |
| 3. Ocean Star | Hyper Eramo (Sea Monster) | Water | Swimming charge |
| 4. Machine Star | Hyper Tom-kun (Pirate Jet/Robo) | Lightning | Transform between jet/robot, breathes flame |
| 5. Gamble Star | Casino Queen (Bunny chariot) | Star | Rides Yuni-pulled slot machine chariot |
| 6. Clouds Star | Hyper UFO | Star | Moves high sideways then dives low |
| 7. Giant Star | Mechazaurus | Fire | Giant robot, ground pound |
| 8. Rainbow Star | Master of Darkness | Water | Fires bubbles to trap player |

## Design

### Boss Enemy Base Class
```cpp
class BossEnemy : public Enemy {
    int maxHealth;          // Total hit points (strength in blue balls)
    int currentHealth;
    PickupType weakness;    // Elemental weakness (mapped to pickup/weapon type)
    bool isDefeated;
    float defeatTimer;      // Animation timer after defeat

    virtual void BossAttack(float delta) = 0;  // Per-boss attack pattern
    virtual void OnHit(int damage);             // Reduce health, check defeat
    virtual void OnDefeat();                    // Trigger victory sequence
};
```

### Boss Level Flow
1. Level loads → background darkens
2. Elemental flask spawns near player spawn point
3. Boss spawns at designated position
4. Player fights boss using elemental weapon
5. Boss defeated → background brightens dramatically
6. Super treat (3-4x normal size) falls from sky center
7. Small treats appear across level
8. If player has 3 Star Mystery Crests → secret door appears

### Master of Darkness (World 8 Final Boss)
- Giant armored Bubble Dragon form (XXL, ~4x player radius)
- Fires bubbles that trap the player (like a stun effect)
- Weakness: Water drops (our existing water weapon!)
- Strength: 50 (requires many hits or water weapon)
- On defeat: victory sequence + potential secret world access

## Implementation Steps

1. **BossEnemy base class** — `BossEnemy.h/.cpp`:
   - Derives from Enemy, adds health bar, weakness type
   - `OnHit()`: reduce health, flash white, check defeat
   - `OnDefeat()`: trigger victory animation sequence
   - Override `Render()` to draw health bar above boss

2. **Boss level detection** — In `GameScreen::LoadLevel()`:
   - Parse `"boss"` JSON key from level data
   - If present: set `isBossLevel = true`, darken background
   - Spawn elemental flask pickup at player spawn

3. **MasterOfDarkness** — First concrete boss:
   - Attack: periodically fire bubble projectiles (reuse Projectile class)
   - Movement: slow drift toward player, occasional charge
   - Weakness: Water weapon deals 10x damage
   - Health: 50

4. **Victory sequence** — On boss defeat:
   - Background brightens (lerp to white-ish)
   - Super treat spawns at level center (3x treat size)
   - Many small treats spawn across level
   - After delay → trigger LEVEL_COMPLETE

5. **Boss health bar HUD** — Draw above boss:
   - Red bar with black border
   - Shows boss name text

## Files to Create
- `game/Umbrella/BossEnemy.h`
- `game/Umbrella/BossEnemy.cpp`
- `game/Umbrella/MasterOfDarkness.h`
- `game/Umbrella/MasterOfDarkness.cpp`

## Files to Modify
- `game/Umbrella/GameScreen.h` — `isBossLevel`, boss pointer, victory state
- `game/Umbrella/GameScreen.cpp` — boss level flow, victory sequence
- `game/Umbrella/MapSerializer.h/cpp` — parse boss data from level JSON
- `game/Umbrella/Umbrella.upp` — add new files

## Dependencies
- D2 (Water weapon) — DONE
- D6 (Level goal system) — DONE
- D9 (Elemental system) — should be done first for weakness mechanic

## Acceptance Criteria
- [ ] Boss level detects "boss" key in level JSON
- [ ] Background darkens at boss level start
- [ ] Boss has health bar displayed above it
- [ ] Boss takes damage from elemental weapon (water for Master of Darkness)
- [ ] Boss defeated triggers victory sequence (bright background, treats)
- [ ] Health bar depletes visually as boss takes damage
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
