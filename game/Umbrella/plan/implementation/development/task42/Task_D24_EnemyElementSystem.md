# Task D24: Enemy Element System — Elemental Interactions

## Priority: LOW — Tier 3 (advanced mechanic)

## Overview

Enemies can have elemental types (None, Water, Ice, Fire, Lightning) that
affect interactions with the player's elemental weapons. An enemy's element
determines its weakness and resistance.

## Java Reference
- `gameplay/enemies/EnemyElementType.java` (NONE, RAINBOW, ICE, FIRE, LIGHTNING)
- `gameplay/enemies/EnemyElementInteractionSystem.java`

## Design

### Element Interaction Matrix
| Enemy Element | Water | Fire | Lightning | Star |
|--------------|-------|------|-----------|------|
| None | Normal | Normal | Normal | Normal |
| Water | Resist | Weak | Normal | Normal |
| Fire | Weak | Resist | Normal | Normal |
| Ice | Normal | Weak | Normal | Normal |
| Lightning | Normal | Normal | Resist | Weak |

- **Weak**: 2x damage, special death animation
- **Resist**: 0.5x damage, deflect visual
- **Normal**: 1x damage

### Per-Enemy Element Assignment
Stored in level JSON per enemy spawn:
```json
{ "col": 5, "row": 10, "type": "PATROLLER", "element": "FIRE" }
```

Visual indicator: colored glow/aura around enemy matching their element.

## Implementation Steps

1. Add `EnemyElement` enum to `Enemy.h`
2. Add element field to `EnemySpawnPoint`
3. Damage multiplier lookup in enemy hit logic
4. Element glow rendering around enemies
5. Serialize/deserialize element in level JSON
6. Editor: element dropdown in enemy placement tool

## Files to Modify
- `game/Umbrella/Enemy.h/cpp` — element field, damage multiplier
- `game/Umbrella/GameScreen.cpp` — element interaction on hit
- `game/Umbrella/MapSerializer.cpp` — serialize element
- `game/Umbrella/EntityPlacementTool.cpp` — element selector in editor

## Dependencies
- D19 (Elemental System) — elemental weapons must exist first

## Acceptance Criteria
- [ ] Enemies can have elemental type
- [ ] Weak element deals 2x damage
- [ ] Resistant element deals 0.5x damage
- [ ] Element glow visible on enemies
- [ ] Element editable in map editor
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
