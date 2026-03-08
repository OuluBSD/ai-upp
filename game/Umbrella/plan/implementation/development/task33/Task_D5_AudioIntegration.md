# Task D5: Audio Integration

## Priority: MEDIUM — Tier 2

## Overview
`AudioSystem.cpp` exists but is not called from gameplay code. Wire it into
key gameplay events so the game has auditory feedback.

## Dependencies
None — `AudioSystem` is self-contained.

## Current State
- `AudioSystem.cpp` compiles but `Play()` / `PlayMusic()` are not called from
  `Player`, `Enemy`, or `GameScreen`
- No sound asset files yet; placeholders (silence or simple sine tones) are
  acceptable initially

## Event → Sound Mapping

| Event | Sound ID |
|---|---|
| Player jump | `"jump"` |
| Player land | `"land"` |
| Player hit / damaged | `"hit"` |
| Player parasol attack | `"whoosh"` |
| Enemy defeated | `"defeat"` |
| Treat collected | `"collect"` |
| Droplet collected | `"droplet"` |
| Level complete | `"victory"` |
| Game over | `"gameover"` |

## Implementation Steps

1. **AudioSystem API** — Verify (or add) `Play(const char* id)` and
   `PlayMusic(const char* id)` methods on `AudioSystem`.

2. **Pass AudioSystem reference** — `GameScreen` owns the `AudioSystem`
   instance; pass it by pointer/reference to `Player` constructor or via a
   setter so `Player` can call `Play("jump")` directly.

3. **Wire gameplay events**:
   - `Player::Jump()` → `audio->Play("jump")`
   - `Player::Land()` → `audio->Play("land")`
   - `Player::TakeDamage()` → `audio->Play("hit")`
   - `Player::Attack()` → `audio->Play("whoosh")`
   - `GameScreen` on enemy defeat → `audio.Play("defeat")`
   - `GameScreen` on treat collect → `audio.Play("collect")`
   - `GameScreen` on level complete → `audio.Play("victory")`

4. **Placeholder assets** — Create 0-byte `.wav` stubs so the paths don't
   crash; or guard with `if(asset_exists(...))`.

## Files to Modify
- `game/Umbrella/AudioSystem.cpp/.h` — verify API, add `Play(const char*)`
- `game/Umbrella/Player.h/.cpp` — accept `AudioSystem*`, call on events
- `game/Umbrella/GameScreen.cpp` — wire defeat/collect/level events

## Acceptance Criteria
- No crashes when audio assets are absent (graceful no-op)
- Audio calls present at all listed gameplay events (verified by log or stub)
