# Next Implementation Tasks

**Status**: Ready for implementation
**Date**: 2026-02-17 (updated)

---

## Completed

1. ✓ **Track 6.2**: Parasol Attack System
2. ✓ **Track 6.3**: Rewards/Treats System
3. ✓ **Track 2 (AI)**: A* Pathfinder + NavGraph + AIController + EnemyBehaviors
4. ✓ **Task D2**: Droplet System Overhaul + Water Weapon (Track 6.1)
5. ✓ **Task D3**: Lives + proper game over
6. ✓ **Task D4**: Pickup System + Flying Enemy (Track 6.4, 7.1)
7. ✓ **Task D5**: GrimReaper boss (Track 7.1)
8. ✓ **Task D6**: Level Goal System (Track 10.2)
9. ✓ **Task D15**: Debug Overlay (Track 11.1)
10. ✓ **Task D16**: Level Select Screen (Track 10.1)
11. ✓ **Task D17**: Pickup Spawn Placement Tool (Track 9.1)

---

## Remaining Tasks — All with plan files in `plan/implementation/development/`

### Tier 1 — HIGH priority (core gameplay + testing)

| Task | File | Description |
|------|------|-------------|
| **D18** | task36/ | Boss System — End-of-world final bosses (Master of Darkness) |
| **D19** | task37/ | Elemental System — Fire/Lightning/Star super powers |
| **D20** | task38/ | Sprite/Animation System — Replace colored rectangles |
| **D25** | task43/ | Gameplay Automation — .ugui/.ugame constraint checking |
| **D29** | task47/ | Enemy Deactivation — Two-stage kill (deactivate → dead) |

### Tier 2 — MEDIUM priority (polish + content)

| Task | File | Description |
|------|------|-------------|
| **D21** | task39/ | Texture Catalog — Sprite management |
| **D22** | task40/ | Audio Integration — Sound effects + music |
| **D23** | task41/ | Additional Enemy Types — Spawner, Idle, Charger |
| **D24** | task42/ | Enemy Element System — Elemental interactions |
| **D26** | task44/ | Entity Editor — Visual sprite/animation editing |

### Tier 3 — LOW priority (backlog)

| Task | File | Description |
|------|------|-------------|
| **D27** | task45/ | Particle Effects — Visual polish |
| **D28** | task46/ | Camera Effects — Screen shake + transitions |
| **D30** | task48/ | Secret World System — Star Crests + endings |

---

## Recommended Implementation Order

```
D29 (Deactivation) → D19 (Elements) → D18 (Boss) → D25 (Automation)
                                                         ↓
D20 (Sprites) → D21 (Textures) → D26 (Entity Editor)
                                                         ↓
D22 (Audio) → D23 (Enemy Types) → D24 (Enemy Elements)
                                                         ↓
D27 (Particles) → D28 (Camera) → D30 (Secret Worlds)
```

D29 first because deactivation is a core combat mechanic that affects how
enemies, droplets, parasol, and bosses all interact. D19 before D18 because
bosses need elemental weaknesses. D25 (automation) should be done early to
enable automated testing of all subsequent features.

---

See `MissingFeatures.md` for full roadmap.
See each task file for detailed implementation plan.
