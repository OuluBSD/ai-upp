# Task D20: Sprite/Animation System — Replace Colored Rectangles

## Priority: HIGH — Tier 1 (visual quality)

## Overview

All game entities currently render as colored rectangles. This task creates a
data-driven sprite and animation system that loads entity definitions from JSON,
manages sprite sheets, and plays frame-based animations.

## Java Reference
- `editor/entity/EntityDefinition.java`
- `editor/entity/EntityAnimation.java`
- `editor/entity/EntityFrame.java`
- `editor/entity/EntitySerializer.java`

## Design

### EntityDefinition (JSON format)
```json
{
  "name": "player",
  "spriteSheet": "sprites/player.png",
  "frameWidth": 16,
  "frameHeight": 16,
  "animations": {
    "idle": { "frames": [0, 1], "duration": 0.5, "loop": true },
    "walk": { "frames": [2, 3, 4, 5], "duration": 0.1, "loop": true },
    "jump": { "frames": [6], "duration": 0.0, "loop": false },
    "attack": { "frames": [8, 9, 10], "duration": 0.08, "loop": false },
    "glide": { "frames": [7], "duration": 0.0, "loop": false }
  },
  "hitbox": { "x": 2, "y": 0, "w": 12, "h": 16 }
}
```

### SpriteSheet Class
```cpp
class SpriteSheet {
    Image sheet;
    int frameWidth, frameHeight;
    int columns;  // Computed: sheet.GetWidth() / frameWidth

    Image GetFrame(int index) const;
    void Load(const String& path, int fw, int fh);
};
```

### AnimationPlayer Class
```cpp
class AnimationPlayer {
    const EntityDefinition* def;
    String currentAnim;
    int currentFrame;
    float frameTimer;
    bool finished;

    void Play(const String& animName);
    void Update(float delta);
    Image GetCurrentFrame() const;
    bool IsFinished() const;
};
```

### EntityDefinition Registry
```cpp
class EntityDefRegistry {
    VectorMap<String, EntityDefinition> defs;

    void LoadAll(const String& defsDir);  // Load all .json in directory
    const EntityDefinition* Get(const String& name) const;
};
```

## Implementation Steps

1. **SpriteSheet class** — `SpriteSheet.h/.cpp`:
   - Load PNG sprite sheet
   - Extract individual frames by index
   - Cache extracted frames

2. **EntityDefinition struct** — `EntityDefinition.h`:
   - Parse from JSON (sprite path, frame size, animations, hitbox)
   - Store animation map (name → frame list + duration + loop flag)

3. **AnimationPlayer class** — `AnimationPlayer.h/.cpp`:
   - Tracks current animation, frame index, timer
   - `Update(delta)` advances frame timer
   - `GetCurrentFrame()` returns Image for current frame
   - `Play(name)` switches animation (resets if different)

4. **EntityDefRegistry** — `EntityDefRegistry.h/.cpp`:
   - Scans directory for entity JSON files
   - Loads and caches all definitions
   - Provides lookup by name

5. **Integration with GameEntity** — Add optional `AnimationPlayer*` to `GameEntity`:
   - If present, `Render()` uses sprite frame instead of colored rect
   - Entities that have sprite definitions use them; others fall back to rectangles

6. **Player integration** — Wire animation states:
   - idle, walk, jump, fall, attack, glide
   - Flip sprite horizontally based on facing direction

7. **Enemy integration** — Wire animation states:
   - idle, walk, jump, attack, deactivated, dead

## Files to Create
- `game/Umbrella/SpriteSheet.h` / `SpriteSheet.cpp`
- `game/Umbrella/EntityDefinition.h` / `EntityDefinition.cpp`
- `game/Umbrella/AnimationPlayer.h` / `AnimationPlayer.cpp`
- `game/Umbrella/EntityDefRegistry.h` / `EntityDefRegistry.cpp`
- `share/mods/umbrella/entities/player.json` (example definition)

## Files to Modify
- `game/Umbrella/GameEntity.h` — optional AnimationPlayer pointer
- `game/Umbrella/Player.cpp` — animation state machine
- `game/Umbrella/Enemy.cpp` — animation state machine
- `game/Umbrella/GameScreen.cpp` — load entity registry on init
- `game/Umbrella/Umbrella.upp` — add new files

## Dependencies
- None (can be done independently)

## Acceptance Criteria
- [ ] SpriteSheet loads PNG and extracts frames by index
- [ ] EntityDefinition parses from JSON with animations
- [ ] AnimationPlayer advances frames at correct speed
- [ ] Player renders with sprite frames instead of colored rectangle
- [ ] Sprites flip horizontally based on facing direction
- [ ] Entities without sprite definitions still render as colored rectangles
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
