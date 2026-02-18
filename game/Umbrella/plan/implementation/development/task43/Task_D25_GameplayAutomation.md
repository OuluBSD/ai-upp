# Task D25: Gameplay Automation — .ugui Constraint Checking for Game

## Priority: HIGH — Tier 1 (testing infrastructure)

## Overview

The codebase has a GUI automation constraint system (`.ugui` files) that uses
first-order logic to verify UI state invariants. This task extends that system
to support gameplay state verification and automated game playthrough testing.

## Existing System

The `.ugui` format is YAML-based:
```yaml
constraints:
  - "VISIBLE(mainWindow) and ENABLED(button)"
  - "CHECKED(option) implies VISIBLE(panel)"
```

Predicates: `VISIBLE`, `ENABLED`, `BUTTON`, `LABEL`, `CHECKED`, `OPTION`,
`HasValue`, `IS_QTF`, plus `RIBBON_*` variants.

Loaded via: `--ugui /path/to/constraints.ugui`
Checked on timer (~1s interval) by `ConstraintVisitor` + `TheoremProver`.

## New Gameplay Predicates

### Game State Predicates
```
GAME_STATE(playing)
GAME_STATE(paused)
GAME_STATE(game_over)
GAME_STATE(level_complete)
GAME_STATE(score_summary)
```

### Player Predicates
```
PLAYER_ALIVE
PLAYER_LIVES(n)           -- n lives remaining
PLAYER_HEALTH(n)          -- current health
PLAYER_GROUNDED            -- on ground
PLAYER_JUMPING             -- in air
PLAYER_ATTACKING           -- parasol active
PLAYER_GLIDING             -- parasol glide
PLAYER_POSITION(col, row) -- grid position
```

### Enemy Predicates
```
ENEMY_COUNT(n)             -- n enemies alive
ALL_ENEMIES_DEAD           -- triggers level complete
ENEMY_EXISTS(type)         -- enemy of given type exists
BOSS_ALIVE                 -- boss is still alive
BOSS_HEALTH(n)             -- boss health percentage
```

### Collectible Predicates
```
DROPLETS_COLLECTED(n)      -- n droplets on umbrella
HAS_HUGE_DROPLET           -- 5+ merged
WATER_WEAPON_ACTIVE        -- water snake active
SCORE(n)                   -- current score >= n
```

### Level Predicates
```
LEVEL_PATH(path)           -- current level file
LEVEL_ELEMENT(type)        -- current level element type
IS_BOSS_LEVEL              -- boss level active
```

## Gameplay Actions (for automated playthrough)

Beyond passive constraint checking, automated gameplay needs **input injection**:

```yaml
actions:
  - { wait: 1.0 }                    # Wait 1 second
  - { move: "right", duration: 2.0 } # Hold right for 2s
  - { jump: true }                    # Press jump
  - { attack: true }                  # Press attack
  - { release: "attack" }            # Release attack button
  - { wait_until: "ALL_ENEMIES_DEAD" } # Wait for condition
  - { assert: "PLAYER_ALIVE" }       # Check condition, fail if false
```

### Automated Playthrough Script (.ugame)
```yaml
name: "World 1 Level 1 Playthrough"
level: "share/mods/umbrella/levels/world1-stage1.json"
timeout: 120  # seconds

steps:
  - { assert: "GAME_STATE(playing)" }
  - { move: "right", duration: 3.0 }
  - { jump: true }
  - { wait: 0.5 }
  - { move: "left", duration: 1.0 }
  - { attack: true }
  - { wait: 0.3 }
  - { release: "attack" }
  - { wait_until: "ALL_ENEMIES_DEAD", timeout: 30 }
  - { assert: "GAME_STATE(level_complete)" }

constraints:
  - "GAME_STATE(playing) implies PLAYER_ALIVE"
  - "PLAYER_LIVES(n) implies n > 0"
```

## Implementation Steps

### Part 1: GameState Fact Extraction

1. **GameStateVisitor** — `GameStateVisitor.h/.cpp`:
   - Walks GameScreen state and produces fact set
   - Maps game state enum to predicate strings
   - Extracts player, enemy, collectible state
   - Produces facts compatible with TheoremProver

2. **Register new predicates** with ConstraintVisitor system:
   - Add gameplay predicates alongside existing GUI predicates
   - GameScreen provides facts when constraint check is triggered

3. **`.ugui` for GameScreen** — Create constraint files:
   ```yaml
   constraints:
     - "GAME_STATE(playing) implies PLAYER_ALIVE"
     - "ALL_ENEMIES_DEAD implies GAME_STATE(level_complete) or GAME_STATE(score_summary)"
     - "PLAYER_LIVES(0) implies GAME_STATE(game_over)"
   ```

### Part 2: Input Injection (Automated Playthrough)

4. **InputInjector** — `InputInjector.h/.cpp`:
   - Queue of timed input actions
   - Overrides `inputState` in GameScreen during automation
   - Supports: move, jump, attack, release, wait, wait_until

5. **`.ugame` script parser** — Parse playthrough scripts
   - YAML format with steps and constraints
   - Load via `--ugame /path/to/script.ugame`

6. **Automated test runner**:
   - Load level, run script, check constraints
   - Report pass/fail with timing info
   - Can run headless (no window) for CI

### Part 3: Constraint Files for Each Level

7. Create `.ugui` files per level with invariants:
   - `share/mods/umbrella/levels/world1-stage1.ugui`
   - Each defines expected game behavior for that level

8. Create `.ugame` playthrough scripts:
   - At least one per world (level X.1) as smoke test
   - Boss level scripts with elemental weapon usage

## Files to Create
- `game/Umbrella/GameStateVisitor.h` / `GameStateVisitor.cpp`
- `game/Umbrella/InputInjector.h` / `InputInjector.cpp`
- `game/Umbrella/GameplayAutomation.h` / `GameplayAutomation.cpp`
- `share/mods/umbrella/levels/*.ugui` (constraint files)
- `share/mods/umbrella/tests/*.ugame` (playthrough scripts)

## Files to Modify
- `game/Umbrella/GameScreen.h/cpp` — expose state for visitor, input injection hook
- `game/Umbrella/main.cpp` — `--ugame` command line flag
- `game/Umbrella/Umbrella.upp` — add new files

## Dependencies
- Existing TheoremProver / ConstraintVisitor infrastructure
- D6 (Level goal system) — DONE
- D18 (Boss system) — for boss level automation

## Acceptance Criteria
- [ ] GameStateVisitor extracts game facts (player, enemies, state)
- [ ] `.ugui` constraints can reference gameplay predicates
- [ ] Constraint violations display red banner in-game
- [ ] InputInjector can drive player movement/actions
- [ ] `.ugame` scripts can automate level playthrough
- [ ] At least one automated playthrough completes World 1 Level 1
- [ ] `--ugame` flag runs automation from command line
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
