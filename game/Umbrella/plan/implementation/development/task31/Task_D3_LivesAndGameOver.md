# Task D3: Lives System + Game Over Flow

## Priority: HIGH — Tier 2 (polish)

## Overview
`Player` already tracks `lives` (default 3) and `GameScreen` has `GAME_OVER`
state. What is missing is the actual flow: decrement lives on death, show
lives in HUD, display a game-over overlay, and return to menu on confirmation.

## Dependencies
- Task D1 (Level Progression) — game-over overlay reuses the same overlay
  rendering approach

## Current State
- `Player::lives` field exists; no code decrements it on death
- `GAME_OVER` state exists; `Paint()` does not render an overlay for it
- Player death currently calls `RespawnPlayer()` immediately (infinite lives)

## Implementation Steps

1. **Decrement on death** — In `GameScreen::HandlePlayerDeath()` (or wherever
   `RespawnPlayer()` is called), first decrement `player.lives`. If lives > 0
   respawn; else call `SetGameState(GAME_OVER)`.

2. **HUD lives display** — In `RenderHUD()`, draw `lives` as a row of small
   colored hearts (rectangles for now) in the top-left, left of the score.

3. **GAME_OVER overlay** — In `Paint()`, when state is `GAME_OVER` render:
   - Semi-transparent dark fill
   - "GAME OVER"
   - Final score
   - "Press Enter to return to menu" hint

4. **Input in GAME_OVER state** — On Enter/Space, call
   `SetGameState(MAIN_MENU)` or equivalent to return to `MainMenuScreen`.

## Files to Modify
- `game/Umbrella/GameScreen.cpp` — death handling, overlay, input
- `game/Umbrella/GameScreen.h` — no new fields needed (lives is on Player)

## Acceptance Criteria
- Dying with lives remaining respawns player; HUD heart count decreases
- Dying on last life shows GAME OVER overlay with final score
- Pressing Enter on GAME OVER returns to main menu
