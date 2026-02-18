# Task D28: Camera Effects — Screen Shake and Transitions

## Priority: LOW — Tier 3 (game feel / juice)

## Overview

Add camera effects for impactful moments: screen shake on player hit, boss
attacks, enemy defeat, and smooth camera transitions.

## Java Reference
- `camera/CameraSystem.java`

## Design

### Screen Shake
```cpp
struct CameraShake {
    float intensity;    // Pixel offset magnitude
    float duration;     // Total shake time
    float timer;        // Remaining time
    float frequency;    // Shake speed (Hz)

    Point GetOffset() const;  // Random offset based on intensity * (timer/duration)
    void Update(float delta);
    bool IsActive() const;
};
```

### Shake Events
| Event | Intensity | Duration |
|-------|-----------|----------|
| Player hit | 4 px | 0.3 s |
| Player death | 8 px | 0.5 s |
| Enemy defeat | 2 px | 0.15 s |
| Boss hit | 3 px | 0.2 s |
| Boss defeat | 12 px | 1.0 s |
| Water weapon destroy | 6 px | 0.4 s |
| Lightning weapon | 3 px | 0.3 s |

### Camera Transitions
- Smooth lerp when following player (already partially implemented)
- Brief freeze-frame on boss defeat (50ms pause)
- Zoom pulse on level complete (slight zoom in then back)

## Implementation Steps

1. **CameraShake struct** — Add to `GameScreen.h`
2. **Apply shake offset** in `UpdateCamera()` / `Paint()`
3. **Trigger shake** at event points in `GameTick()`
4. **Freeze-frame** — Skip N frames on boss defeat for impact
5. **Zoom pulse** — Brief zoom change on level complete

## Files to Modify
- `game/Umbrella/GameScreen.h` — CameraShake member
- `game/Umbrella/GameScreen.cpp` — shake triggers, apply in Paint()

## Acceptance Criteria
- [ ] Screen shakes on player hit
- [ ] Stronger shake on boss defeat
- [ ] Shake intensity decays over duration
- [ ] No shake during menus/pause
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
