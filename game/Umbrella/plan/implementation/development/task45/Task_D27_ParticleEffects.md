# Task D27: Particle Effects — Visual Polish

## Priority: LOW — Tier 3 (visual polish)

## Overview

Add particle systems for key game events: enemy defeat, droplet collection,
player hit, parasol swing, level complete celebration, elemental weapons.

## Design

### ParticleSystem Class
```cpp
struct Particle : Moveable<Particle> {
    float x, y;
    float vx, vy;
    float life, maxLife;
    Color color;
    float size;
};

class ParticleEmitter {
    Vector<Particle> particles;
    float gravity;
    float fadeRate;

    void Emit(float x, float y, int count, Color color, float speed, float spread);
    void Update(float delta);
    void Render(Draw& w, Player::CoordinateConverter& coords);
};
```

### Particle Events
| Event | Particle Style |
|-------|---------------|
| Enemy defeat | Orange/red explosion burst (20 particles) |
| Enemy deactivate | Green sparkle ring (10 particles) |
| Droplet collect | Blue sparkle upward (8 particles) |
| Treat collect | Yellow coins upward (6 particles) |
| Player hit | Red flash + white impact stars (12 particles) |
| Parasol swing | White trail arc (6 particles) |
| Level complete | Multi-color confetti (50 particles) |
| Water weapon | Blue splash at each step (4 particles) |
| Fire weapon | Orange embers at flame positions (3 per flame) |
| Lightning weapon | Yellow sparks along beam (continuous) |

## Implementation Steps

1. **ParticleEmitter class** — `ParticleSystem.h/.cpp`
2. **Integration** — Add emitter to GameScreen, trigger on events
3. **Per-event configuration** — Color, count, speed, gravity, spread
4. **Performance** — Cap total particles at 200, oldest-first removal

## Files to Create
- `game/Umbrella/ParticleSystem.h` / `ParticleSystem.cpp`

## Files to Modify
- `game/Umbrella/GameScreen.h/cpp` — particle emitter, trigger calls

## Acceptance Criteria
- [ ] Particles spawn on enemy defeat
- [ ] Particles spawn on droplet/treat collection
- [ ] Level complete triggers confetti
- [ ] Performance stays acceptable with particle cap
- [ ] Build: `script/build.py -mc 1 -j 12 Umbrella`
