# Task D1: Level Complete + Next-Level Progression

## Priority: HIGH — Tier 1 (core game loop)

## Overview
The game currently loops forever with no win condition. When all enemies are
dead the `LEVEL_COMPLETE` state is set, but no overlay is shown and no next
level is loaded. This task completes the loop: clear screen → score summary →
load next level (or victory screen if none remain).

## Dependencies
- Task A4 (Level loading) — `LevelLoader.cpp` already loads a single file
- Treats/score system — already complete

## Current State
- `GameScreen` has `LEVEL_COMPLETE` state and `SetGameState(LEVEL_COMPLETE)`
  called when enemy count reaches zero (`GameScreen.cpp` ~line 362)
- `levelCompleteTimer` exists for a brief delay after last enemy dies
- No overlay rendered for `LEVEL_COMPLETE`; no level list / next-file logic

## Implementation Steps

1. **Level list** — Add a simple ordered list of level files to `GameScreen`
   (or a plain text manifest). Start with the existing single level; add more
   as they are created.
   ```cpp
   // GameScreen.h
   Vector<String> levelList;
   int            currentLevelIdx = 0;
   ```

2. **Render LEVEL_COMPLETE overlay** — In `GameScreen::Paint`, when state is
   `LEVEL_COMPLETE`, draw a semi-transparent dark overlay and display:
   - "LEVEL CLEAR"
   - Score earned this level
   - Continue hint (any key / Enter)

3. **Input in LEVEL_COMPLETE state** — On keypress, advance to
   `currentLevelIdx + 1`. If past end of list, show `VICTORY` screen and
   return to main menu.

4. **LoadNextLevel()** helper — Clears enemies, treats, droplets; calls
   `LoadLevel(levelList[currentLevelIdx])`.

5. **GAME_OVER overlay** — Already has state; add a rendered overlay ("GAME
   OVER", score, restart/menu options) so the state is visually handled.

## Files to Modify
- `game/Umbrella/GameScreen.h` — `levelList`, `currentLevelIdx`
- `game/Umbrella/GameScreen.cpp` — overlay rendering, input handling,
  `LoadNextLevel()`

## Files to Create
None — all changes fit in existing files.

## Acceptance Criteria
- Defeating all enemies shows "LEVEL CLEAR" overlay with score
- Pressing Enter/Space loads the next level file (or returns to menu)
- `GAME_OVER` state shows an overlay instead of a blank game
