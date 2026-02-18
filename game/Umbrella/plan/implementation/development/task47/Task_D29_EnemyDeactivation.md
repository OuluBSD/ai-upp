# Task D29: Enemy Deactivation — Two-Stage Kill Mechanic

## Priority: HIGH — Tier 1 (core combat mechanic)

## Overview

In the original game, enemies don't die instantly. They have a two-stage death:
1. **Deactivated**: Enemy is stunned, turns greenish, can be picked up
2. **Dead**: Enemy is killed, turns reddish, flies in arc, lands as treat

This creates depth: players can deactivate enemies with small droplets (1-4),
then throw them at other enemies, or let the water weapon carry them.

Deactivated enemies left too long become **zombies** (red, 2-3x speed, start jumping).

## Java Reference
- `~/Dev/RainbowGame/trash/USER_EXPERIENCE.md` section 3.1.2

## Source Material

From USER_EXPERIENCE.md:
- Enemy strength is measured in blue balls needed to deactivate
- Deactivated: greenish tint, spins around, floats
- Dead: red tint, flies in direction, lands on ground → becomes treat
- Zombie: enemy left deactivated too long, turns red, moves 2-3x speed
- Deactivated enemy on umbrella → throw horizontally → kills on wall hit
- Thrown deactivated enemy takes other enemies with it

### Strength by Size
- XS: 1 blue ball
- S: 2
- M: 3
- L: 4
- XL: 5
- XXL: 50 (boss)

## Design

### Enemy States
```cpp
enum EnemyState {
    ENEMY_ALIVE,        // Normal, moves with AI
    ENEMY_DEACTIVATED,  // Stunned, floats, can be picked up
    ENEMY_ZOMBIE,       // Deactivated too long, fast + aggressive
    ENEMY_ON_UMBRELLA,  // Riding player's umbrella
    ENEMY_THROWN,        // Flying horizontally after throw
    ENEMY_DEAD          // Red tint, arc to ground, becomes treat
};
```

### Deactivation Timer
```cpp
static constexpr float ZOMBIE_TIMER = 8.0f;  // Seconds until zombie
static constexpr float ZOMBIE_WARNING = 3.0f; // Red glow warning before zombie
```

### Umbrella Capture
When parasol hits deactivated enemy (size <= M):
- Enemy attaches to umbrella
- Moves with player
- On action release → thrown horizontally in facing direction
- Thrown enemy kills on wall contact
- Takes other enemies of same size or smaller with it

## Implementation Steps

1. **Enemy state machine** — Add states to Enemy class
2. **Deactivation** — Blue ball hits reduce strength counter
3. **Visual feedback** — Green tint when deactivated, red glow before zombie
4. **Zombie transformation** — Timer-based, increased speed + jumping
5. **Umbrella capture** — Parasol hit on deactivated enemy attaches to player
6. **Throw mechanic** — Release action throws captured enemy horizontally
7. **Chain kills** — Thrown enemy sweeps same-size-or-smaller enemies

## Files to Modify
- `game/Umbrella/Enemy.h/cpp` — state machine, deactivation, zombie
- `game/Umbrella/Player.h/cpp` — captured enemy tracking
- `game/Umbrella/GameScreen.cpp` — deactivation logic, throw, chain kills

## Dependencies
- D2 (Droplet system) — DONE (provides blue ball hits)

## Acceptance Criteria
- [ ] Enemies deactivate (not die) when hit by droplets matching their strength
- [ ] Deactivated enemies have green tint and float
- [ ] Deactivated enemies become zombies after 8 seconds
- [ ] Zombies move faster and start jumping
- [ ] Player can capture deactivated enemies on umbrella
- [ ] Releasing action throws captured enemy horizontally
- [ ] Thrown enemy kills on wall contact, sweeps smaller enemies
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
