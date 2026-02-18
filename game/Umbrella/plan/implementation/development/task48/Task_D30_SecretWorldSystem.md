# Task D30: Secret World System — Star Mystery Crests + Endings

## Priority: LOW — Tier 3 (endgame content)

## Overview

The game has multiple endings based on collectible "Star Mystery Crests" and
a "Giant Key". Collecting 3 crests + key unlocks access to Worlds 9-10 and
the true ending. Without them, the player gets the bad ending after World 8.

## Java Reference
- `~/Dev/RainbowGame/trash/USER_EXPERIENCE.md` sections 3.1.2, 3.1.3

## Source Material

From USER_EXPERIENCE.md:
- Players collect **element-cube sets** throughout the game
- Goal: collect 3 elements (displayed on screen HUD)
- Certain combinations are "key sets" that unlock secret functions
- 3 **Star Mystery Crests** + **Giant Key** = access to World 9 + 10
- After World 8 boss defeat, if crests collected → **secret door** appears
- Player enters door → space map shows new planets (World 9: Bubble Star,
  World 10: Underworld)
- Without crests → **bad ending** after World 8

### Worlds
- 0-8: Regular progression (Rainbow → Music → ... → Rainbow Star)
- 9: Bubble Star (secret)
- 10: Underworld (secret, final)

## Design

### Collectible Tracking
```cpp
struct PlayerProgress {
    int starCrests;          // 0-3 Star Mystery Crests collected
    bool hasGiantKey;        // Giant Key obtained
    Vector<String> elements; // Element cubes collected (display in HUD)
    int worldsCompleted;     // Highest world beaten
};
```

### Secret Door
- Spawns at level center after boss defeat in World 8
- Only if `starCrests >= 3 && hasGiantKey`
- Player walks into door → triggers World 9 transition
- Visual: glowing portal/door sprite

### Endings
1. **Bad ending**: Defeat World 8 boss without crests → credits roll
2. **Good ending**: Enter secret door → World 9 → World 10 → true credits
3. **True ending**: Complete World 10 final boss → extended ending sequence

### Element Cube Spawning
- Hidden in specific level locations (secret areas)
- 3 per world, one of each element type relevant to that world
- Displayed as blue squares with element icon in center

## Implementation Steps

1. **PlayerProgress** — Track crests and key in GameScreen/save data
2. **Element cube collectible** — New pickup type, spawn in levels
3. **HUD display** — Show collected elements (3 slots) in corner
4. **Secret door** — Spawn after boss defeat if criteria met
5. **Bad ending screen** — Simple ending screen after World 8
6. **World 9-10 levels** — Level data for secret worlds
7. **Space map animation** — Between-world transition showing planets

## Files to Create
- `game/Umbrella/PlayerProgress.h` / `PlayerProgress.cpp`
- `game/Umbrella/SpaceMap.h` / `SpaceMap.cpp` (between-world animation)

## Files to Modify
- `game/Umbrella/GameScreen.h/cpp` — progress tracking, secret door
- `game/Umbrella/Pickup.h` — add element cube pickup type

## Dependencies
- D18 (Boss system) — boss defeat triggers secret door
- D19 (Elemental system) — element cubes reference elements

## Acceptance Criteria
- [ ] Element cubes spawn in hidden level locations
- [ ] HUD shows collected element cubes
- [ ] Defeating World 8 boss with 3 crests + key spawns secret door
- [ ] Entering secret door transitions to World 9
- [ ] Without crests → bad ending screen
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
