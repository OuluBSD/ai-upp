# Task D22: Audio Integration — Sound Effects + Music

## Priority: MEDIUM — Tier 2 (game feel / polish)

## Overview

AudioSystem.cpp exists but is not integrated into gameplay. This task hooks
sound effects into game events and adds background music playback.

## Java Reference
- `assets/AudioSystem.java`

## Design

### Sound Event Map
| Event | Sound | Trigger Location |
|-------|-------|-----------------|
| Player jump | jump.wav | Player.cpp |
| Player land | land.wav | Player.cpp |
| Player hit | hit.wav | GameScreen.cpp |
| Player death | death.wav | GameScreen.cpp |
| Enemy defeat | enemy_defeat.wav | GameScreen.cpp |
| Enemy deactivated | deactivate.wav | GameScreen.cpp |
| Treat collect | coin.wav | GameScreen.cpp |
| Droplet collect | droplet.wav | GameScreen.cpp |
| Parasol attack | whoosh.wav | Player.cpp |
| Parasol glide | glide_loop.wav | Player.cpp |
| Level complete | victory.wav | GameScreen.cpp |
| Boss hit | boss_hit.wav | GameScreen.cpp |
| Boss defeat | boss_defeat.wav | GameScreen.cpp |
| Water weapon | water_flow.wav | WaterWeapon.cpp |
| GrimReaper warning | reaper_warning.wav | GameScreen.cpp |
| Menu select | menu_select.wav | MainMenuScreen.cpp |

### Background Music
- Per-world music track (loops)
- Boss music (darker, more intense)
- GrimReaper override music (urgent)
- Level complete jingle (one-shot)

## Implementation Steps

1. **Sound catalog** — Map event IDs to file paths in `AudioSystem`
2. **Play calls** — Add `AudioSystem::Play("event_id")` calls at trigger points
3. **Background music** — `AudioSystem::PlayMusic(path, loop)` / `StopMusic()`
4. **Volume control** — Global volume, SFX/music separate volumes
5. **Placeholder sounds** — Generate simple sine/square wave tones as placeholders

## Files to Modify
- `game/Umbrella/AudioSystem.h/cpp` — add Play/PlayMusic/StopMusic
- `game/Umbrella/Player.cpp` — jump, land, attack, glide sounds
- `game/Umbrella/GameScreen.cpp` — defeat, collect, level complete sounds
- `game/Umbrella/WaterWeapon.cpp` — water flow sound
- `game/Umbrella/MainMenuScreen.cpp` — menu sounds

## Dependencies
- None (AudioSystem infrastructure already exists)

## Acceptance Criteria
- [ ] Jump sound plays on every jump
- [ ] Enemy defeat plays sound
- [ ] Droplet/treat collection plays sound
- [ ] Background music loops per level
- [ ] Boss music plays on boss levels
- [ ] GrimReaper warning sound plays
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
