# Task D4: Pickup System

## Priority: MEDIUM — Tier 2

## Overview
Spawn collectible pickups in levels that give the player immediate or timed
benefits: HEART (restore life), GEM (bonus score), LIGHTNING (temporary
invincibility), SPEED (movement speed boost).

## Dependencies
- Task D3 (Lives system) — HEART pickup requires `player.lives` increment

## Java Reference
`gameplay/components/Pickup.java`

## Implementation Steps

1. **`Pickup` class** — `Pickup.h/.cpp`:
   ```cpp
   enum PickupType { PU_HEART, PU_GEM, PU_LIGHTNING, PU_SPEED };
   class Pickup {
       Rectf bounds;
       PickupType type;
       bool active;
       float bobTimer;  // sinusoidal vertical bob for visibility
   public:
       Pickup(float x, float y, PickupType t);
       void Update(float delta);
       void Render(Draw& w, Player::CoordinateConverter& coords);
       bool IsActive() const { return active; }
       void Collect() { active = false; }
       Rectf GetBounds() const { return bounds; }
       PickupType GetType() const { return type; }
   };
   ```

2. **Spawn in GameScreen** — Hardcode a few pickup positions per level (same
   pattern as enemies/emitters). Add `Array<Pickup*> pickups` to `GameScreen`.

3. **Collision + effects** — In `GameTick`, check each pickup against player:
   - `PU_HEART`: `player.lives = min(player.lives + 1, MAX_LIVES)`
   - `PU_GEM`: `player.AddScore(100)`
   - `PU_LIGHTNING`: set `player.invincibleTimer = 5.0f`
   - `PU_SPEED`: set `player.speedBoostTimer = 10.0f`

4. **Timed effects on Player** — Add `invincibleTimer` and `speedBoostTimer`
   float fields to `Player`; tick them down each frame; apply speed multiplier
   (1.5×) when boost active; skip projectile-hit damage when invincible.

5. **Rendering** — Color-coded rectangles:
   - HEART = red, GEM = cyan, LIGHTNING = yellow, SPEED = orange

## Files to Create
- `game/Umbrella/Pickup.h`
- `game/Umbrella/Pickup.cpp`

## Files to Modify
- `game/Umbrella/GameScreen.h/.cpp` — pickup array, spawn, collision
- `game/Umbrella/Player.h/.cpp` — `invincibleTimer`, `speedBoostTimer`
- `game/Umbrella/Umbrella.upp` — add Pickup.h / Pickup.cpp

## Acceptance Criteria
- Pickups visible in level, bob gently up/down
- Each pickup type applies correct effect on collection
- Timed effects expire after their duration
- Invincibility prevents projectile damage
